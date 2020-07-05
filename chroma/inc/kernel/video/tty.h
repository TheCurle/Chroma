#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

void Term_Init(void);
void Term_PutChar(char c);
void Term_Write(const char* str, size_t len);
void Term_WriteStr(const char* str);

#endif
