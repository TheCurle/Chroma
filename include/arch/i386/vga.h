#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H

#include <stdint.h>

enum vga_colors {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GREY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	LIGHT_MAGENTA = 13,
	LIGHT_BROWN = 14,
	WHITE = 15
};

static inline uint8_t vga_color_set(enum vga_colors fg, enum vga_colors bg) { return bg << 4 | fg; }

static inline uint16_t vga_entry(uint8_t  uc, uint8_t color) { return  color << 8 |  uc; }

#endif
