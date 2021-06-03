/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/
 
 /* This file provides all of the functionality
  * needed to interact with the Text-Mode VGA
  * buffer, which will soon be defunct due to
  * EFI and graphics.
  * 
  * This file will be left for the forseeable
  * future, because it allows for easy debugging.
  * 17/07/19 Curle
  */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//#include <string.h>

#include "kernel/tty.h"
#include "kernel/utils.h"

static const size_t TERM_WIDTH = 80;
static const size_t TERM_HEIGHT = 25;

static size_t terminal_row;
static size_t terminal_column;

static uint8_t current_color;
static uint16_t* term_buffer;
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;

void screen_initialize(void) {

	terminal_row = 0;
	terminal_column = 0;
	current_color = vga_color_set(LIGHT_GREY, BLACK);
	term_buffer = vga_buffer;

	for (size_t y = 0; y < TERM_HEIGHT; y++) {
		for (size_t x = 0; x < TERM_WIDTH; x++) {
			const size_t offset = y * TERM_WIDTH + x;
			term_buffer[offset] = vga_entry(' ', current_color);
		}
	}
}

void term_setcolor(enum vga_colors color) { current_color = color; }

void term_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t offset = y * TERM_WIDTH + x;
	term_buffer[offset] = vga_entry(c, color);
}

void term_putchar(char c) {
	unsigned char uc = c;

	// Handle escaped characters, such as newline, and crtn.
	switch (uc) {
	case '\n':
		terminal_column = 0;
		terminal_row += 1;
		break;
	default:
		term_putentryat(uc, current_color, terminal_column, terminal_row);
		if (++terminal_column == TERM_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == TERM_HEIGHT) {
				term_scroll(false);
				terminal_row = 0;
			}
		}
		break;
	}
}

struct csi_sequence parse_csi(const char* data, size_t size) {
	enum State { PARAMETER, INTERMEDIATE, FINAL, INVALID };
	enum State state = PARAMETER;

	struct csi_sequence sequence = {.parameter = NULL,
	                                .parameter_len = 0,
	                                .intermediate = NULL,
	                                .intermediate_len = 0,
	                                .final = NULL,
	                                .valid = false};

	for (size_t j = 0; j < size; j++) {
		uint8_t c = data[j];
		if (state == PARAMETER && (c >= 0x30 && c <= 0x3F)) {
			if (!sequence.parameter)
				sequence.parameter = data + j;
			sequence.parameter_len++;
		} else if (c >= 0x20 && c <= 0x2F) {
			if (!sequence.intermediate)
				sequence.intermediate = data + j;
			sequence.intermediate_len++;
			state = INTERMEDIATE;
		} else if (c >= 0x40 && c <= 0x7F) {
			sequence.final = data + j;
			sequence.valid = true;
			state = FINAL;
			break;
		} else {
			// Parameter found in intermediate byte location, or byte out of
			// range
			state = INVALID;
			break;
		}
	}
	return sequence;
}

void term_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		// Begin handling ANSI escape codes.
		if (data[i] == 0x1b) { // The current character is ESC - the start of ANSI codes.
			// term_writes("ANSI Code encountered: ");

			bool string_terminated = false; // Flag used in some of the escape codes

			// TODO: Should only progress if we have at least 2 more bytes

			switch ((uint8_t)data[i + 1]) {
			case '[': // CSI - Control Sequence Introducer (The most common one, hence it comes first)
			{
				struct csi_sequence sequence = parse_csi(data + i + 2, size - (i + 2));
				if (sequence.valid) {
					// Send it off to our handler function to keep this part clean
					handleControlSequence(sequence);
					i += sequence.parameter_len + sequence.intermediate_len + 2; // Move past sequence
				}
			} break;
			// Single shifts are not handled, so just print them and exit
			case 'N': // SS2 - Single Shift Two
				term_writes("Single Shift Two\n");
				break;
			case 'O': // SS3 - Single Shift Three
				term_writes("Single Shift Three\n");
				break;

			// Control Strings
			case 'P': // DCS - Device Control String
				term_writes("Device Control String");
				string_terminated = false;
				break;
			case '\\': // ST - String Terminator
				term_writes("String Terminator\n");
				string_terminated = true;
				break;
			case ']': // OSC - Operating System Command
				term_writes("Operating System Command\n");
				string_terminated = false;
				break;
			case 'X': // SOS - Start Of String
				term_writes("Start of String");
				string_terminated = false;
				break;
			case '^': // PM - Privacy Message
				term_writes("Privacy Message\n");
				break;
			case '_': // APC - Application Program Command
				term_writes("Application Program Command\n");
				break;
			}
		} else {
			term_putchar(data[i]);
		}
	}
}

void term_writes(const char* data) { term_write(data, strlen(data)); }

void puts(const char* string) {
	term_write(string, strlen(string));
	term_putchar('\n');
}

void handleControlSequence(struct csi_sequence sequence) {
	// Check for our failsafes

	if (sequence.valid) {
		int n = 0; // Default of the flag used for a few items

		// Parse parameters, we only care about a max 2 of number only parameters
		// for now
		int params[2] = {0};
		int param_count = 0;
		if (sequence.parameter_len) {
			for (size_t i = 0; i < sequence.parameter_len && param_count < 1; i++) {
				char c = sequence.parameter[i];
				if (isDigit(c)) {
					n = (n * 10) + (sequence.parameter[i] - '0');
				} else if (c == ';') {
					params[param_count++] = n;
				}
			}
			params[param_count++] = n;
		}

		switch (*(sequence.final)) {
		case 'H':
		case 'f': // CUP - Cursor Position
			// TODO: Check to see if we have 2 paramaters
			if (params[0])
				params[0]--;
			if (params[1])
				params[1]--;
			set_cursor(params[0], params[1]);
			break;
		case 'A': // CUU - Cursor Up
			if (!params[0])
				params[0] = 1;
			set_cursor(terminal_column, terminal_row - params[0]);
			break;
		case 'B': // CUD - Cursor Down
			if (!params[0])
				params[0] = 1;
			set_cursor(terminal_column, terminal_row + params[0]);
			break;
		case 'C': // CUF - Cursor Forward
			if (!params[0])
				params[0] = 1;
			set_cursor(terminal_column + params[0], terminal_row);
			break;
		case 'D': // CUB - Cursor Back
			if (!params[0])
				params[0] = 1;
			set_cursor(terminal_column - params[0], terminal_row);
			break;
		case 'E': // CNL - Cursor Next Line
			if (!params[0])
				params[0] = 1;
			set_cursor(0, terminal_row + params[0]);
			break;
		case 'F': // CPL - Cursor Previous Line
			if (!params[0])
				params[0] = 1;
			set_cursor(0, terminal_row - params[0]);
			break;
		case 'G': // CHA - Cursor Horizontal Absolute
			if (params[0])
				params[0]--;
			set_cursor(params[0], terminal_row);
			break;
		case 'J': // ED - Erase in Display
		{
			// current cursor pos = y * width + x
			int pos = terminal_row * 80 + terminal_column;
			if (params[0] == 0) { // Clear from cursor to end of screen
				for (; pos < (25 * 80); pos++) {
					vga_buffer[pos] = '\0';
				}
			} else if (params[0] == 1) { // Clear from cursor to beginning
				for (; pos > 0; pos--) {
					vga_buffer[pos] = '\0';
				}
			} else if (params[0] == 2 || params[0] == 3) { // Clear entire screen
				// TODO: Support scrollback buffer? (n = 3)
				for (int i = 0; i < (25 * 80); i++) {
					vga_buffer[0] = '\0';
				}
			}
			break;
		}
		case 'K': // EL - Erase in Line
		{
			int pos = terminal_row * 80 + terminal_column;
			if (params[0] == 0) {                       // From cursor to end of line
				int endPos = (terminal_row + 1) * 80 - 1; // End of line = current row + 25 columns = current row + 1
				for (; pos < endPos; pos++) {
					vga_buffer[pos] = '\0';
				}
			} else if (params[0] == 1) {      // From cursor to start of line
				int endPos = terminal_row * 80; // Start of line = end of previous line + 1 == current line
				for (; pos > endPos; pos--) {
					vga_buffer[pos] = '\0';
				}
			} else if (params[0] == 2) { // Entire current line
				pos = terminal_row * 80;
				int endPos = (terminal_row + 1) * 80 - 1;
				for (; pos < endPos; pos++) {
					vga_buffer[pos] = '\0';
				}
			}
			break;
		}
		case 'S': // SU - Scroll Up
			term_scroll(true);
			break;
		case 'T': // SD - Scroll Down
			term_scroll(false);
			break;
		}
	}
}

bool isDigit(char c) { return c >= '0' && c <= '9'; }

void set_cursor(int n, int m) {
	terminal_column = n;
	terminal_row = m;
}

void term_scroll(bool down) {

	int current_pos;
	if (down) {
		current_pos = 25 * 80; // Start of the last line.
	} else {
		current_pos = 160; // Start of the second line.
	}

	unsigned char* term_buffer = (unsigned char*)vga_buffer;

	if (down) { // To scroll down, move every character into the one below it, or "pull" every character down
		while (current_pos > 80) {
			term_buffer[current_pos + 160] = vga_buffer[current_pos];
			term_buffer[current_pos + 159] = vga_buffer[current_pos - 1];
			current_pos -= 2;
		}
	} else {
		while (current_pos <= 4000) {
			term_buffer[current_pos - 160 /*The character immediately below it*/] =
			    vga_buffer[current_pos]; // Move each character up a line
			term_buffer[current_pos - 159 /*The color of the character below*/] =
			    vga_buffer[current_pos + 1]; // As well as its color
			current_pos += 2;                // Move to next char
		}
	}

	if (down) {
		current_pos = 0; // Start of first line
		for (; current_pos < 80; current_pos++) {
			term_buffer[current_pos] = '\0';
			current_pos += 2;
		}
	} else {
		; // Start of the last line
		// Wipe out the bottom line
		for (current_pos = 3840; current_pos <= 3920; current_pos += 2) {
			term_buffer[current_pos] = 0;
		}
	}
	terminal_row = 24; // Start writing on the last line
	terminal_column = 0;
}
