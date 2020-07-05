#ifndef VGA_TEXTMODE_H
#define VGA_TEXTMODE_H

#include <stdint.h>

enum VGA_COLOR {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHTGREY = 7,
	VGA_COLOR_DARKGREY = 8,
	VGA_COLOR_LIGHTBLUE = 9,
	VGA_COLOR_LIGHTGREEN = 10,
	VGA_COLOR_LIGHTCYAN = 11,
	VGA_COLOR_PINK = 12,
	VGA_COLOR_LIGHTMAGENTA = 13,
	VGA_COLOR_LIGHTBROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t VGACharColor(enum VGA_COLOR fg, enum VGA_COLOR bg) {
	return fg | bg << 4;
}

static inline uint8_t VGAChar(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

#endif

