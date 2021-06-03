#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/i386/vga.h"

struct csi_sequence {
	const char* parameter;
	size_t parameter_len;
	const char* intermediate;
	size_t intermediate_len;
	const char* final;
	bool valid;
};

void term_setcolor(enum vga_colors);
void screen_initialize(void);
void term_putentryat(char, uint8_t, size_t, size_t);
void term_putchar(char);
void term_write(const char*, size_t);
void term_writes(const char*);
void puts(const char*);
void handleControlSequence(struct csi_sequence);
void set_cursor(int, int);
void term_scroll(bool);

bool isDigit(char c);
