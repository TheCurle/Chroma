#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <kernel/system/interrupts.h>
#include <stdbool.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains all of the ISR and IRQ
 * (Interrupt Service Request) functions.
 *
 * As they use the GCC interrupt attribute,
 * this file must be compiled without red-
 * zone protection, thus all of these
 * functions are in their own file to
 * accomodate this.
 *
 * Additionally, the kernel now has SSE/AVX support.
 * So this file and this file *alone* must be compiled with
 * -mgeneral-regs-only
 *
 * Calling a function like so:
 *
 * __attribute__((interrupt)) isr1(registers_t* frame) {}
 *
 * allows the function to be used to serve
 * interrupts - GCC compiles it under unique
 * conditions, as it preserves the state of
 * the processor and stack between execution,
 * as well as using the IRET instruction to
 * return to the middle of the previous function.
 *
 * There is also a version of the interrupt
 * attribute which allows for error handlers,
 * these having a size_t input as an error code.
 */

#ifdef __cplusplus
extern "C" {
#endif

const char* ExceptionStrings[] = {
        "Division by Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",
        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
};

IRQHandlerData IRQHandlers[32];

/* All of the ISR routines call this function for now.
   ! This function is NOT leaf, and it might clobber the stack.
   ! Be careful! 
*/
__attribute__((no_caller_saved_registers)) void ISR_Common(INTERRUPT_FRAME* Frame, size_t Exception) {

    UNUSED(Frame);

    /* Only the first 32 ISR/IRQs are reserved for exceptions by the CPU. We can handle up to 512 interrupts total, though. */
    if (Exception < 32) {
        SetForegroundColor(0x0000FF00);
        FillScreen(0x0000FF00);
        /* ExceptionStrings is an array of c-strings defined in kernel.h */
        SerialPrintf("[  ISR] %s exception!\r\n", ExceptionStrings[Exception]);
    }
}

/* The common handler for exceptions that throw error codes, which give us useful insight
	into what went wrong. In pure Curle style, though, we just ignore the error code. */
void ISR_Error_Common(INTERRUPT_FRAME* Frame, size_t ErrorCode, size_t Exception) {

    UNUSED(Frame);

    if (Exception < 32) {
        SetForegroundColor(0x0000FF00);
        FillScreen(0x0000FF00);
        SerialPrintf("[  ISR] ISR Error %d raised, EC %d!\r\n", Exception, ErrorCode);
        SerialPrintf("[  ISR] %s exception!\r\n", ExceptionStrings[Exception]);
        while (true) { }

    }

}

/* Likewise, this function is common to all IRQ handlers. It calls the assigned routine, 
	which was set up earlier by irq_install.*/
void IRQ_Common(INTERRUPT_FRAME* Frame, size_t Interrupt) {
    // First we need to define a function pointer..
    IRQHandlerData handler;

    /* We set all uninitialized routines to 0, so the if(handler) check here allows us to
        safely tell whether we've actually got something for this IRQ. */
    handler = IRQHandlers[Interrupt];
    if (handler.numHandlers > 0) {
        SerialPrintf("[  IRQ] IRQ %d raised!\r\n", Interrupt);
        // Call the handlers
        for (size_t i = 0; i < handler.numHandlers; i++)
            handler.handlers[i](Frame);
    }

    /* The Slave PIC must be told it's been read in order to receive another 8+ IRQ. */
    if (Interrupt > 7)
        WritePort(0xA0, 0x20, 1);

    /* In either case, we tell the Master PIC it's been read to receive any IRQ. */
    WritePort(0x20, 0x20, 1);
}
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

void ClearIRQ(size_t idx) {
    uint16_t port;
    uint8_t value;

    if(idx < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        idx -= 8;
    }
    value = ReadPort(port, 1) & ~(1 << idx);
    WritePort(port, value, 1);
}

/* However, in order to actually be able to receive IRQs, we need to remap the PICs. */
void RemapIRQControllers() {
    /* 0x20 is the Master PIC,
       0xA0 is the Slave PIC. */
    unsigned char a1, a2;

    a1 = ReadPort(PIC1_DATA, 1);                        // save masks
    a2 = ReadPort(PIC2_DATA, 1);

    WritePort(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4, 1);  // starts the initialization sequence (in cascade mode)
    WritePort(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4, 1);
    WritePort(PIC1_DATA, 32, 1);                 // ICW2: Master PIC vector offset
    WritePort(PIC2_DATA, 32 + 8, 1);             // ICW2: Slave PIC vector offset
    WritePort(PIC1_DATA, 4, 1);                  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    WritePort(PIC2_DATA, 2, 1);                  // ICW3: tell Slave PIC its cascade identity (0000 0010)

    WritePort(PIC1_DATA, ICW4_8086, 1);
    WritePort(PIC2_DATA, ICW4_8086, 1);

    WritePort(PIC1_DATA, a1, 1);   // restore saved masks.
    WritePort(PIC2_DATA, a2, 1);

    ClearIRQ(1);
    ClearIRQ(2);
}

/* In order to actually handle the IRQs, though, we need to tell the kernel *where* the handlers are. */
/* A simple wrapper that adds a function pointer to the IRQ array. */
size_t InstallIRQ(int IRQ, IRQHandler Handler) {
    if (IRQ <= 32) {
        IRQHandlerData* target = &IRQHandlers[IRQ];
        if (target->numHandlers < 7) {
            target->handlers[target->numHandlers] = Handler;
            target->numHandlers++;
            return target->numHandlers;
        }
    }

    return 0;
}

/* A simple wrapper that unlinks a function pointer, rendering the IRQ unused. */
void UninstallIRQHandler(int IRQ, size_t ID) {
    IRQHandlers[IRQ].handlers[ID] = NULL; // 0 is used in the common check to make sure that the function is callable.
    // This removes this IRQ from that check, ergo the function will no longer be called.
}

void InitInterrupts() {
    size_t RFLAGS = ReadControlRegister('f');

    if (!(RFLAGS & (1 << 9))) {
        WriteControlRegister('f', RFLAGS | (1 << 9));
    }

    __asm__ __volatile__("sti");
}


/* The interrupt numbers, their meanings, and
 * special information is laid out below:
 *
 * 0 - Divide by Zero
 * 1 - Debug
 * 2 - Non-Maskable
 * 3 - Breakpoint
 * 4 - Into Detected Overflow
 * 5 - Out of Bounds
 * 6 - Invalid Opcode
 * 7 - No Coprocessor
 * 8 - Double Fault   *                (With Error)
 * 9 - Coprocessor Segment Overrun
 * 10 - Bad TSS       *                (With Error)
 * 11 - Segment Not Present *          (With Error)
 * 12 - Stack Fault   *                (With Error)
 * 13 - General Protection Fault *     (With Error)
 * 14 - Page Fault    *                (With Error)
 * 15 - Unknown Interrupt
 * 16 - Coprocessor Fault
 * 17 - Alignment Check
 * 18 - Machine Check
 * 19 to 31 - Reserved
 */

__attribute__((interrupt)) void ISR0Handler(INTERRUPT_FRAME* Frame) {

    ISR_Common(Frame, 0);
}

__attribute__((interrupt)) void ISR1Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 1);
}

__attribute__((interrupt)) void ISR2Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 2);
}

__attribute__((interrupt)) void ISR3Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 3);
}

__attribute__((interrupt)) void ISR4Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 4);
}

__attribute__((interrupt)) void ISR5Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 5);
}

__attribute__((interrupt)) void ISR6Handler(INTERRUPT_FRAME* Frame) {

    __asm__ __volatile__("sti");

    SerialPrintf("[FAULT] Invalid Opcode!\n");
    size_t retAddr = 0;
    size_t opcodeAddr = Frame->rip;

    __asm__ __volatile__("popq %%rax\n\t" "pushq %%rax": "=a" (retAddr) : :);
    SerialPrintf("[FAULT] Opcode is at 0x%x, called from 0x%p\r\n", opcodeAddr, retAddr);
    Printf("Invalid Opcode: 0x%p\n", opcodeAddr);

    Core::GetCurrent()->StackTrace(15);

    for (;;) { }
}

__attribute__((interrupt)) void ISR7Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 7);
}

__attribute__((interrupt)) void ISR8Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 8);
}

__attribute__((interrupt)) void ISR9Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 9);
}

__attribute__((interrupt)) void ISR10Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 10);
}

__attribute__((interrupt)) void ISR11Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 11);
}

__attribute__((interrupt)) void ISR12Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 12);
}

__attribute__((interrupt)) void ISR13Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {

    SerialPrintf("\r\n\n[  GPF] RIP: 0x%p, CS: 0x%x, FLAGS: 0x%p, RSP: 0x%x, SS: 0x%x\r\n", Frame->rip, Frame->cs,
                 Frame->rflags, Frame->rsp, Frame->ss);

    Core::GetCurrent()->StackTrace(6);

    ISR_Error_Common(Frame, ErrorCode, 13); // General Protection
}

__attribute__((interrupt)) void ISR14Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    __asm__ __volatile__("sti");

    SerialPrintf("\r\n\n\n[FAULT] Page fault! Caused by {\r\n");

    //size_t FaultAddr = ReadControlRegister(2);
    uint8_t FaultPres = ErrorCode & 0x1;
    uint8_t FaultRW = ErrorCode & 0x2;
    uint8_t FaultUser = ErrorCode & 0x4;
    uint8_t FaultReserved = ErrorCode & 0x8;
    uint8_t FaultInst = ErrorCode & 0x10;

    if (!FaultPres) SerialPrintf("[FAULT] Accessed a page that isn't present.\r\n");
    if (FaultRW || FaultUser) SerialPrintf("[FAULT] Accessed a Read-Only page.\r\n");
    if (FaultReserved) SerialPrintf("[FAULT] Overwrote reserved bits.\r\n");
    if (FaultInst) SerialPrintf("[FAULT] \"Instruction Fetch\"");

    SerialPrintf("[FAULT] } at address\n[FAULT] 0x%p\r\n\n", ReadControlRegister(2));

    Core::GetCurrent()->StackTrace(10);
    ISR_Error_Common(Frame, ErrorCode, 14); // Page Fault
}

__attribute__((interrupt)) void ISR15Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 15);
}

__attribute__((interrupt)) void ISR16Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 16);
}

__attribute__((interrupt)) void ISR17Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 17);
}

__attribute__((interrupt)) void ISR18Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 18);
}

__attribute__((interrupt)) void ISR19Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 19);
}

__attribute__((interrupt)) void ISR20Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 20);
}

__attribute__((interrupt)) void ISR21Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 21);
}

__attribute__((interrupt)) void ISR22Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 22);
}

__attribute__((interrupt)) void ISR23Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 23);
}

__attribute__((interrupt)) void ISR24Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 24);
}

__attribute__((interrupt)) void ISR25Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 25);
}

__attribute__((interrupt)) void ISR26Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 26);
}

__attribute__((interrupt)) void ISR27Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 27);
}

__attribute__((interrupt)) void ISR28Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 28);
}

__attribute__((interrupt)) void ISR29Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 29);
}

__attribute__((interrupt)) void ISR30Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
    ISR_Error_Common(Frame, ErrorCode, 30);
}

__attribute__((interrupt)) void ISR31Handler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 31);
}

__attribute__((interrupt)) void ReservedISRHandler(INTERRUPT_FRAME* Frame) {
    ISR_Common(Frame, 33); // if < 32, isn't handled.
    // Effectively disables this ISR.
}


__attribute__((interrupt)) void IRQ0Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 0);
}

__attribute__((interrupt)) void IRQ1Handler(INTERRUPT_FRAME* Frame) {
    SerialPrintf("IRQ1\r\n");
    IRQ_Common(Frame, 1); // Keyboard handler
}

__attribute__((interrupt)) void IRQ2Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 2);
}

__attribute__((interrupt)) void IRQ3Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 3);
}

__attribute__((interrupt)) void IRQ4Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 4);
}

__attribute__((interrupt)) void IRQ5Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 5);
}

__attribute__((interrupt)) void IRQ6Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 6);
}

__attribute__((interrupt)) void IRQ7Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 7);
}

__attribute__((interrupt)) void IRQ8Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 8);
}

__attribute__((interrupt)) void IRQ9Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 9);
}

__attribute__((interrupt)) void IRQ10Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 10);
}

__attribute__((interrupt)) void IRQ11Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 11);
}

__attribute__((interrupt)) void IRQ12Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 12);
}

__attribute__((interrupt)) void IRQ13Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 13);
}

__attribute__((interrupt)) void IRQ14Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 14);
}

__attribute__((interrupt)) void IRQ15Handler(INTERRUPT_FRAME* Frame) {
    IRQ_Common(Frame, 15);
}

#ifdef __cplusplus
}
#endif