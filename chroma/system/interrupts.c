/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
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

#include <kernel/chroma.h>
#include <kernel/system/interrupts.h>
#include <stdbool.h>

#define UNUSED(X) ((void)X)

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

typedef void (*IRQHandler)(INTERRUPT_FRAME* Frame);

IRQHandler IRQ_Handlers[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


typedef unsigned long long int uword_t;

/* All of the ISR routines call this function for now.
   ! This function is NOT leaf, and it might clobber the stack.
   ! Be careful! 
*/
void ISR_Common(INTERRUPT_FRAME* Frame, size_t Exception) {

	UNUSED(Frame);

	/* Only the first 32 ISR/IRQs are reserved for exceptions by the CPU. We can handle up to 512 interrupts total, though. */
	if(Exception < 32) {

		FillScreen(0x0000FF00);
		/* ExceptionStrings is an array of c-strings defined in kernel.h */
		
		SerialPrintf("%s exception!\r\n", ExceptionStrings[Exception]);
		//printf("%s exception!", ExceptionStrings[Exception]);
		//panic();
	}
}

/* The common handler for exceptions that throw error codes, which give us useful insight
	into what went wrong. In pure Curle style, though, we just ignore the error code. */
void ISR_Error_Common(INTERRUPT_FRAME* Frame, size_t ErrorCode, size_t Exception) {

	UNUSED(Frame);

	if(Exception < 32) {

		FillScreen(0x0000FF00);
		
		SerialPrintf("ISR Error %d raised, EC %d!\r\n", Exception, ErrorCode);
		SerialPrintf("%s exception!\r\n", ExceptionStrings[Exception]);
		while(true) {}
		//serialPrint(ExceptionStrings[Exception]);
		//serialPrintf(" Exception. Context given: %d\r\n", Frame->ErrorCode);
		//printf("%s exception. Context: %x", ExceptionStrings[Exception], Frame->ErrorCode);
		//panic();

	}

}

/* Likewise, this function is common to all IRQ handlers. It calls the assigned routine, 
	which was set up earlier by irq_install.*/
void IRQ_Common(INTERRUPT_FRAME* Frame, size_t Interrupt) {
	// First we need to define a function pointer..
	void (*Handler)(INTERRUPT_FRAME* Frame);

	// Tell the user we've got an interrupt in..
	//serial_print(0x3F8, "[INFO] Received IRQ: " + interrupt);
	//printf("[INFO] Received IRQ: %x", Interrupt);

	/* We set all uninitialized routines to 0, so the if(handler) check here allows us to
		safely tell whether we've actually got something for this IRQ. */
	Handler = IRQ_Handlers[Interrupt];
	// If there's something there,
	if(Handler) {
		SerialPrintf("IRQ %d raised!\r\n", Interrupt);
		// Call the handler.
		Handler(Frame);
	}
	/* The Slave PIC must be told it's been read in order to receive another 8+ IRQ. */
	if(Interrupt > 7)
		WritePort(0xA0, 0x20, 1);
	
	/* In either case, we tell the Master PIC it's been read to receive any IRQ. */
	WritePort(0x20, 0x20, 1);
}

/* However, in order to actually be able to receive IRQs, we need to remap the PICs. */
void RemapIRQControllers() {
	/* 0x20 is the Master PIC,
	   0xA0 is the Slave PIC. */
	WritePort(0x20, 0x11, 1);
	WritePort(0xA0, 0x11, 1);
	WritePort(0x21, 0x20, 1);
	WritePort(0xA1, 0x28, 1);
	WritePort(0x21, 0x04, 1);
	WritePort(0xA1, 0x02, 1);
	WritePort(0x21, 0x01, 1);
	WritePort(0xA1, 0x01, 1);
	WritePort(0x21, 0x0, 1);
	WritePort(0xA1, 0x0, 1);
}

/* In order to actually handle the IRQs, though, we need to tell the kernel *where* the handlers are. */
/* A simple wrapper that adds a function pointer to the IRQ array. */
void InstallIRQ(int IRQ, void (*Handler)(INTERRUPT_FRAME* Frame)) {
	IRQ_Handlers[IRQ] = Handler;
}

/* A simple wrapper that unlinks a function pointer, rendering the IRQ unused. */
void UninstallIRQHandler(int IRQ) {
	IRQ_Handlers[IRQ] = NULL; // 0 is used in the common check to make sure that the function is callable.
	// This removes this irq from that check, ergo the function will no longer be called.
}

void EmptyIRQ(INTERRUPT_FRAME* frame) {

	UNUSED(frame);

	// Flash the borders green, then back to blue

	for(size_t y = 0; y < bootldr.fb_height; y++) {
		for(size_t x = 0; x < 20; x++) {
			DrawPixel(x, y, 0x0000FF00);
		}

		for(size_t x = (bootldr.fb_width - 20); x < bootldr.fb_width; x++) {
			DrawPixel(x, y, 0x0000FF00);
		}
	}

	for(size_t x = 0; x < bootldr.fb_width; x++) {
		for(size_t y = 0; y < 20; y++) {
			DrawPixel(x, y, 0x0000FF00);
		}

		for(size_t y = (bootldr.fb_height - 20); y < bootldr.fb_height; y++) {
			DrawPixel(x, y, 0x0000FF00);
		}
	}
	
	for(size_t y = 0; y < bootldr.fb_height; y++) {
		for(size_t x = 0; x < 20; x++) {
			DrawPixel(x, y, 0x000000FF);
		}

		for(size_t x = (bootldr.fb_width - 20); x < bootldr.fb_width; x++) {
			DrawPixel(x, y, 0x000000FF);
		}
	}

	for(size_t x = 0; x < bootldr.fb_width; x++) {
		for(size_t y = 0; y < 20; y++) {
			DrawPixel(x, y, 0x000000FF);
		}

		for(size_t y = (bootldr.fb_height - 20); y < bootldr.fb_height; y++) {
			DrawPixel(x, y, 0x000000FF);
		}
	}
}

static void KeyboardCallback(INTERRUPT_FRAME* frame) {

	UNUSED(frame);

	uint8_t msg = ReadPort(0x60, 1);

	UpdateKeyboard(msg);

	WaitFor8042();
}

void InitInterrupts() {
	size_t RFLAGS = ReadControlRegister('f');

	if(!(RFLAGS & (1 << 9))) {
		WriteControlRegister('f', RFLAGS | (1 << 9));
	}


	InstallIRQ(1, &KeyboardCallback);

	Send8042(0xF002);

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
	ISR_Common(Frame, 6);
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
	ISR_Error_Common(Frame, ErrorCode, 13); // General Protection
}
__attribute__((interrupt)) void ISR14Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode) {
	__asm__ __volatile__("sti");

	SerialPrintf("Page fault! Caused by: [\r\n");

	//size_t FaultAddr = ReadControlRegister(2);
	uint8_t FaultPres = 	ErrorCode & 0x1;
	uint8_t FaultRW = 		ErrorCode & 0x2;
	uint8_t FaultUser = 	ErrorCode & 0x4;
	uint8_t FaultReserved = ErrorCode & 0x8;
	uint8_t FaultInst = 	ErrorCode & 0x10;

	if(!FaultPres) SerialPrintf("Accessed a page that isn't present.\r\n");
	if(FaultRW || FaultUser) SerialPrintf("Accessed a Read-Only page.\r\n");
	if(FaultReserved) SerialPrintf("Overwrote reserved bits.\r\n");
	if(FaultInst) SerialPrintf("\"Instruction Fetch\"");

	SerialPrintf("];");	

	
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