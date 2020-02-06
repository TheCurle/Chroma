/************************
 *** Team Kitty, 2020 ***
 ***       Sync       ***
 ***********************/

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <kernel/boot/boot.h>
#include <kernel/system/io.h>
#include <kernel/system/memory.h>

extern bootinfo bootldr;
extern unsigned char* environment;
extern uint8_t fb;


void DrawPixel(uint32_t x, uint32_t y, uint32_t color);
void FillScreen(uint32_t color);

void WriteString(const char* string);
void WriteChar(const char character);

void InitInterrupts();
void InitSerial();
void InitPrint();

void SetupInitialGDT();
void SetupIDT();
