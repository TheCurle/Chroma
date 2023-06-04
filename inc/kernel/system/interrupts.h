#pragma once
#include <stddef.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

 #ifdef __cplusplus
extern "C" {
#endif

    // We need this to work for process switching now, so have all 64 bit registers in here.
typedef struct __attribute__((packed)) {
    size_t r15;
    size_t r14;
    size_t r13;
    size_t r12;
    size_t r11;
    size_t r10;
    size_t r9;
    size_t r8;
    size_t rbp;
    size_t rdi;
    size_t rsi;
    size_t rdx;
    size_t rcx;
    size_t rbx;
    size_t rax;

    size_t inter; // Interrupt number
    size_t err;   // Error number - 0 if no error.

    size_t  rip;
    size_t  cs;
    size_t  rflags;
    size_t  rsp;
    size_t  ss;
} InterruptFrame;


typedef void (*IRQHandler)(InterruptFrame* Frame);

typedef struct  {
    IRQHandler handlers[8];
    size_t numHandlers;
} IRQHandlerData;

extern IRQHandlerData IRQHandlers[32];

size_t InstallIRQ(int IRQ, IRQHandler handler);
void UninstallIRQHandler(int IRQ, size_t ID);

void IRQ_Common(InterruptFrame* Frame, size_t Interupt);

#ifdef __cplusplus
}
#endif