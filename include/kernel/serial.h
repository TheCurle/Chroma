#pragma once
#include <stdint.h>
#include <stddef.h>

void init_serial();

void serial_set_baud_rate(uint16_t, uint16_t);

void serial_configure_line(uint16_t);

void serial_configure_buffers(uint16_t);

void serial_write(uint16_t, const char);

void serial_configure_modem(uint16_t);

int serial_check_tqueue(uint16_t);

void serial_print(uint16_t, const char*);

void serial_printf(uint16_t, const char*, ...);
