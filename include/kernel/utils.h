
#pragma once
#include <stddef.h>

/* A temporary file, to get the system compiling. */

size_t strlen(const char*);

unsigned char inb(unsigned short);

void outb(unsigned short, unsigned char);

void memcpy(void*, void*, size_t);

void memset(void*, int, size_t);

void int_to_ascii(int, char*);

void int_to_hex(int, char*);

void empty_string(char*);

char* itoc(size_t);
