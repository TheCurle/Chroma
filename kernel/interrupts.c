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

#include <kernel.h>

typedef unsigned long long int uword_t;


static void* IRQ_Handlers[16] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


/* All of the ISR routines call this function for now.
   ! This function is NOT leaf, and it might clobber the stack.
   ! Be careful! 
*/
void ISR_Common(INTERRUPT_FRAME* Frame, size_t Exception) {
	/* Only the first 32 ISR/IRQs are reserved for exceptions by the CPU. We can handle up to 512 interrupts total, though. */
	if(Exception < 32) {
		/* ExceptionStrings is an array of c-strings defined in kernel.h */
		
		serialPrint(ExceptionStrings[Exception]);
		serialPrint(" Exception.\r\n");
		printf("%s exception!", ExceptionStrings[Exception]);
		panic();
	}
}

/* The common handler for exceptions that throw error codes, which give us useful insight
	into what went wrong. In pure Curle style, though, we just ignore the error code. */
void ISR_Error_Common(EXCEPTION_FRAME* Frame, size_t Exception) {
	if(Exception < 32) {
		serialPrint(ExceptionStrings[Exception]);
		serialPrintf(" Exception. Context given: %d\r\n", Frame->ErrorCode);
		printf("%s exception. Context: %x", ExceptionStrings[Exception], Frame->ErrorCode);
		panic();
	}

}

/* Likewise, this function is common to all IRQ handlers. It calls the assigned routine, 
	which was set up earlier by irq_install.*/
void IRQ_Common(INTERRUPT_FRAME* Frame, size_t Interrupt) {
	// First we need to define a function pointer..
	void (*Handler)(INTERRUPT_FRAME* Frame);

	// Tell the user we've got an interrupt in..
	//serial_print(0x3F8, "[INFO] Received IRQ: " + interrupt);
	printf("[INFO] Received IRQ: %x", Interrupt);

	/* We set all uninitialized routines to 0, so the if(handler) check here allows us to
		safely tell whether we've actually got something for this IRQ. */
	Handler = IRQ_Handlers[Interrupt];
	// If there's something there,
	if(Handler) 
		// Call the handler.
		Handler(Frame);
	
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
void InstallIRQHandler(int IRQ, void (*Handler)(INTERRUPT_FRAME* Frame)) {
	IRQ_Handlers[IRQ] = Handler;
}

/* A simple wrapper that unlinks a function pointer, rendering the IRQ unused. */
void UninstallIRQHandler(int IRQ) {
	IRQ_Handlers[IRQ] = 0; // 0 is used in the common check to make sure that the function is callable.
	// This removes this irq from that check, ergo the function will no longer be called.
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
	// This baby is the timer.
	// TODO: Change ISR0Handler to TimerHandler.
	// Here, we can do some timer related stuff.
	// For starts, we just tick a timer that tells us how long the program has been running.
	time++;
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
__attribute__((interrupt)) void ISR8Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 8);
}
__attribute__((interrupt)) void ISR9Handler(INTERRUPT_FRAME* Frame) {
	ISR_Common(Frame, 9);
}
__attribute__((interrupt)) void ISR10Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 10);
}
__attribute__((interrupt)) void ISR11Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 11);
}
__attribute__((interrupt)) void ISR12Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 12);
}
__attribute__((interrupt)) void ISR13Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 13);
}
__attribute__((interrupt)) void ISR14Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 14);
}
__attribute__((interrupt)) void ISR15Handler(INTERRUPT_FRAME* Frame) {
	ISR_Common(Frame, 15);
}
__attribute__((interrupt)) void ISR16Handler(INTERRUPT_FRAME* Frame) {
	ISR_Common(Frame, 16);
}
__attribute__((interrupt)) void ISR17Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 17);
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
__attribute__((interrupt)) void ISR30Handler(EXCEPTION_FRAME* Frame) {
	ISR_Error_Common(Frame, 30);
}
__attribute__((interrupt)) void ISR31Handler(INTERRUPT_FRAME* Frame) {
	ISR_Common(Frame, 31);
}

__attribute__((interrupt)) void ReservedISRHandler(INTERRUPT_FRAME* Frame) {
	ISR_Common(Frame, 33); // if < 32, isn't handled.
	// Effectively disables this ISR.
}


__attribute__((interrupt)) void irq0(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 0);
}
__attribute__((interrupt)) void irq1(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 1);
}
__attribute__((interrupt)) void irq2(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 2);
}
__attribute__((interrupt)) void irq3(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 3);
}
__attribute__((interrupt)) void irq4(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 4);
}
__attribute__((interrupt)) void irq5(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 5);
}
__attribute__((interrupt)) void irq6(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 6);
}
__attribute__((interrupt)) void irq7(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 7);
}
__attribute__((interrupt)) void irq8(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 8);
}
__attribute__((interrupt)) void irq9(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 9);
}
__attribute__((interrupt)) void irq10(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 10);
}
__attribute__((interrupt)) void irq11(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 11);
}
__attribute__((interrupt)) void irq12(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 12);
}
__attribute__((interrupt)) void irq13(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 13);
}
__attribute__((interrupt)) void irq14(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 14);
}
__attribute__((interrupt)) void irq15(INTERRUPT_FRAME* Frame) {
	IRQ_Common(Frame, 15);
}