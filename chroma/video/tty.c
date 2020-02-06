#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <kernel/video/tty.h>

#include <kernel/video/vga-textmode.h>

static const size_t TERM_WIDTH = 80;
static const size_t TERM_HEIGHT = 25;

size_t TERM_ROW;
size_t TERM_COL;
uint8_t TERM_COLOR;

uint16_t* TERM_BUFFER;

void Term_Init(void) {
	
	TERM_ROW = 0;
	TERM_COL = 0;
	TERM_COLOR = VGACharColor(VGA_COLOR_LIGHTGREY, VGA_COLOR_BLACK);

	TERM_BUFFER = (uint16_t*) 0xB8000;

	for(size_t y = 0; y < TERM_HEIGHT; y++) {
		for(size_t x = 0; x < TERM_WIDTH; x++) {
			const size_t index = y * TERM_WIDTH + x;
			TERM_BUFFER[index] = VGAChar(' ', TERM_COLOR);
		}
	}
}

void Term_SetColor(uint8_t color) {
	TERM_COLOR = color;
}

void Term_PutVGAChar(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * TERM_WIDTH + x;
	TERM_BUFFER[index] = VGAChar(c, color);
}

void Term_PutChar(char c) {
	Term_PutVGAChar(c, TERM_COLOR, TERM_COL, TERM_ROW);

	if(++TERM_COL == TERM_WIDTH) {
		TERM_COL = 0;
		if(++TERM_ROW == TERM_HEIGHT)
			TERM_ROW = 0;
	}
}

void Term_Write(const char* str, size_t len) {
	for(size_t i = 0; i < len; i++)
		Term_PutChar(str[i]);
}

void Term_WriteString(const char* str) {
	Term_Write(str, strlen(str));
}

