#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <kernel/system/interrupts.h>
#include <stdbool.h>
#include "driver/io/apic.h"

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains all of the ISR and IRQ
 * (Interrupt Service Request) functions.
 *
 * They are generated with a NASM macro in the global/interrupts.s file, with InterruptsCommon as the entry point.
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

bool isError(size_t inter) {
    if (inter > 31) return false;
    if (inter == 15 || (inter > 20 && inter < 30) || inter == 31) return false;
    return true;
}

[[noreturn]] void InterruptErrorCommon(InterruptFrame* frame) {
    // TODO: Reallocate stack on page fault when CR2 > stack address.

    SerialPrintf("[INTER] Fatal interrupt error at 0x%p.\r\n", frame->rip);
    SerialPrintf("[INTER] ID %d\r\n", frame->inter);
    SerialPrintf("[INTER] Name %s\r\n\n", ExceptionStrings[frame->inter]);

    while(true)
        asm("hlt");
}

size_t InterruptCommon(InterruptFrame* frame) {
    auto result = reinterpret_cast<size_t>(frame);
    SerialPrintf("[INTER] Interrupt raised!\r\n");

    if (isError(frame->inter)) InterruptErrorCommon(frame);

    if (frame->inter > 32 && frame->inter < 64)
        IRQ_Common(frame, frame->inter - 32);
    // TODO: Process Switch, Syscall

    Device::APIC::driver->SendEOI();
    return result;
}


/* Likewise, this function is common to all IRQ handlers. It calls the assigned routine, 
	which was set up earlier by irq_install.*/
void IRQ_Common(InterruptFrame* Frame, size_t Interrupt) {
    SerialPrintf("[  IRQ] %u raised\r\n", Interrupt);
    // First we need to define a function pointer..
    IRQHandlerData handler;

    /* We set all uninitialized routines to 0, so the if(handler) check here allows us to
        safely tell whether we've actually got something for this IRQ. */
    handler = IRQHandlers[Interrupt];
    if (handler.numHandlers > 0) {
        SerialPrintf("[  IRQ] IRQ %d raised! %d listeners.\r\n", Interrupt, handler.numHandlers);
        // Call the handlers
        for (size_t i = 0; i < handler.numHandlers; i++)
            handler.handlers[i](Frame);
    }

    WritePort(0x20, 0x20, 1); WritePort(0xA0, 0x20, 1);
}


/* In order to actually handle the IRQs, though, we need to tell the kernel *where* the handlers are. */
/* A simple wrapper that adds a function pointer to the IRQ array. */
size_t InstallIRQ(int IRQ, IRQHandler Handler) {
    if (IRQ <= 32) {
        Device::APIC::driver->Set(0, IRQ, 1);

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
    IRQHandlers[IRQ].handlers[ID] = nullptr; // 0 is used in the common check to make sure that the function is callable.
    // This removes this IRQ from that check, ergo the function will no longer be called.
}

void InitInterrupts() {
    size_t RFLAGS = ReadControlRegister('f');

    if (!(RFLAGS & (1 << 9))) {
        WriteControlRegister('f', RFLAGS | (1 << 9));
    }

    __asm__ __volatile__("sti");
}


/*
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
} */


#ifdef __cplusplus
}
#endif