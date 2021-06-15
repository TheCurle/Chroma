#include <kernel/chroma.h>
/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* 
 * This file aims to implement stack unwinding
 * to trace faulty functions.
 * 
 * I was in the middle of debugging a jump to null
 * when i started creating this, so there will be a
 * lot of functionality here left over from that
 * initial goal, probably...
 * 
 * //TODO: Rework this to allow unwinding function parameters on call.
 */

void StackTrace(size_t cycles) {
    struct stackframe* stack;

    __asm__ __volatile__ ("mov %%rbp, %[dest]" : [dest] "=r" (stack) : :);
    SerialPrintf("[Trace] Beginning stack trace. RBP is currently 0x%p\r\n", stack);
    for(size_t frame = 0; stack != 0 && frame < cycles; ++frame) {
        SerialPrintf("[Trace] (%d) 0x%p:  0x%p \r\n", frame, stack->rbp, stack->rip);
        stack = stack->rbp;
    }
    SerialPrintf("[Trace] Stack trace over.\r\n");
}