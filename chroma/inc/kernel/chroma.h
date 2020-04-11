#pragma once
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

extern size_t LoadAddr;
extern bootinfo bootldr;
extern unsigned char* environment;
extern uint8_t fb;
extern volatile unsigned char _binary_font_psf_start;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headerSize;
    uint32_t flags;
    uint32_t numGlyphs;
    uint32_t glyphSize;
    uint32_t glyphHeight;
    uint32_t glyphWidth;
    uint8_t glyphs;
} __attribute__((packed)) psf_t;


size_t KernelAddr;
size_t KernelEnd;
size_t MemoryPages;
size_t MemorySize;


void DrawPixel(uint32_t x, uint32_t y, uint32_t color);
void FillScreen(uint32_t color);

void WriteString(const char* string);
void WriteChar(const char character);

void WriteStringWithFont(const char* string);

void InitInterrupts();
void InitSerial();
void InitPrint();

void SetupInitialGDT();
void SetupIDT();
