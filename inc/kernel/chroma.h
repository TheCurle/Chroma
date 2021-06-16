#pragma once
/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file serves as the central kernel header. Every file in the kernel should include this header.
 * It provides functionality to every core component of the system, and provides unrestricted cross-communication between modules.
 * It also provides the symbols for the framebuffer and configuration file, which are both equually important.
 */

#define UNUSED(x) (void)x

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <kernel/boot/boot.h>
#include <kernel/system/io.h>
#include <kernel/system/memory.h>
#include <kernel/system/pci.h>
#include <kernel/system/stack.h>

#include <lainlib/lainlib.h>
#include <lainlib/ethernet/e1000/e1000.h>

//Removed cause "wacky copyrighted stuff"
//#include <kernel/system/screen.h>

extern size_t LoadAddr;
extern bootinfo bootldr;
extern unsigned char* environment;
extern uint8_t fb;
extern volatile unsigned char _binary_src_assets_font_psf_start;
extern volatile size_t* _kernel_text_start;

extern address_space_t KernelAddressSpace;

extern e1000_device_t* E1000NIC;

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
size_t FreeMemorySize;
size_t FullMemorySize;


void SetupExtensions();
void PrepareCPU();

void WriteString(const char* string);
void WriteChar(const char character);

void WriteStringWithFont(const char* string);

void InitInterrupts();
void InitSerial();
void InitPrint();

void SetupInitialGDT();
void SetupIDT();

int ParseKernelHeader(size_t InitrdPtr);

int Main();

void Exit();

void SomethingWentWrong(const char* Message);