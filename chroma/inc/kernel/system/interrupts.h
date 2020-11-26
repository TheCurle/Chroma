#pragma once
#include <stddef.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

extern const char* ExceptionStrings[];


typedef struct __attribute__((packed)) {
    size_t  rip;
    size_t  cs;
    size_t  rflags;
    size_t  rsp;
    size_t  ss;
} INTERRUPT_FRAME;

typedef struct __attribute__((packed)) {
    size_t ErrorCode;
    size_t rip;
    size_t cs;
    size_t rflags;
    size_t rsp;
    size_t ss;
} EXCEPTION_FRAME;


typedef void (*IRQHandler)(INTERRUPT_FRAME* Frame);

extern IRQHandler IRQ_Handlers[16];

void IRQ_Common(INTERRUPT_FRAME* Frame, size_t Interupt);
void ISR_Common(INTERRUPT_FRAME* Frame, size_t Interrupt);
void ISR_Error_Common(INTERRUPT_FRAME* Frame, size_t ErrorCode, size_t Exception);

void RemapIRQControllers();

void ISR0Handler(INTERRUPT_FRAME* Frame); // Divide-By-Zero
void ISR1Handler(INTERRUPT_FRAME* Frame); // Debug
void ISR2Handler(INTERRUPT_FRAME* Frame); // Non-Maskable Interrupt
void ISR3Handler(INTERRUPT_FRAME* Frame); // Breakpoint
void ISR4Handler(INTERRUPT_FRAME* Frame); // Overflow
void ISR5Handler(INTERRUPT_FRAME* Frame); // Out-of-Bounds
void ISR6Handler(INTERRUPT_FRAME* Frame); // Invalid Opcode
void ISR7Handler(INTERRUPT_FRAME* Frame); // No Coprocessor
void ISR8Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Double Fault
void ISR9Handler(INTERRUPT_FRAME* Frame); // Coprocessor Overrun
void ISR10Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Invalid TSS
void ISR11Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Segment Not Present
void ISR12Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Stack Segment Fault
void ISR13Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // General Protection Fault
void ISR14Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Page Fault
void ISR15Handler(INTERRUPT_FRAME* Frame); // Unknown Interrupt
void ISR16Handler(INTERRUPT_FRAME* Frame); // Math Error / Coprocessor Fault
void ISR17Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Alignment Error
void ISR18Handler(INTERRUPT_FRAME* Frame); // Machine Check
void ISR19Handler(INTERRUPT_FRAME* Frame); // SSE Math Error
void ISR20Handler(INTERRUPT_FRAME* Frame); // Virtualization 
void ISR21Handler(INTERRUPT_FRAME* Frame);
void ISR22Handler(INTERRUPT_FRAME* Frame);
void ISR23Handler(INTERRUPT_FRAME* Frame);
void ISR24Handler(INTERRUPT_FRAME* Frame);
void ISR25Handler(INTERRUPT_FRAME* Frame);
void ISR26Handler(INTERRUPT_FRAME* Frame);
void ISR27Handler(INTERRUPT_FRAME* Frame);
void ISR28Handler(INTERRUPT_FRAME* Frame);
void ISR29Handler(INTERRUPT_FRAME* Frame);
void ISR30Handler(INTERRUPT_FRAME* Frame, size_t ErrorCode); // Security Fault
void ReservedISRHandler(INTERRUPT_FRAME* Frame);


void IRQ0Handler(INTERRUPT_FRAME* Frame);
void IRQ1Handler(INTERRUPT_FRAME* Frame);
void IRQ2Handler(INTERRUPT_FRAME* Frame);
void IRQ3Handler(INTERRUPT_FRAME* Frame);
void IRQ4Handler(INTERRUPT_FRAME* Frame);
void IRQ5Handler(INTERRUPT_FRAME* Frame);
void IRQ6Handler(INTERRUPT_FRAME* Frame);
void IRQ7Handler(INTERRUPT_FRAME* Frame);
void IRQ8Handler(INTERRUPT_FRAME* Frame);
void IRQ9Handler(INTERRUPT_FRAME* Frame);
void IRQ10Handler(INTERRUPT_FRAME* Frame);
void IRQ11Handler(INTERRUPT_FRAME* Frame);
void IRQ12Handler(INTERRUPT_FRAME* Frame);
void IRQ13Handler(INTERRUPT_FRAME* Frame);
void IRQ14Handler(INTERRUPT_FRAME* Frame);
void IRQ15Handler(INTERRUPT_FRAME* Frame);
