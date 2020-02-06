#include <kernel/chroma.h>

#define SERIAL_DATA(base)           (base)
#define SERIAL_DLAB(base)           (base + 1)
#define SERIAL_FIFO(base)           (base + 2)
#define SERIAL_LINE(base)           (base + 3)
#define SERIAL_MODEM(base)          (base + 4)
#define SERIAL_LINE_STATUS(base)    (base + 5)
#define COM1                        0x3F8

void InitSerial() {
    // Disable interrupts
    WritePort(SERIAL_DLAB(COM1), 0x00, 1);
    // Set baud rate to /3 (115200 bits/sec)
    WritePort(SERIAL_LINE(COM1), 0x80, 1);
    WritePort(SERIAL_DATA(COM1), 0x03, 1);
    WritePort(SERIAL_DLAB(COM1), 0x00, 1);
    // Set serial to 8 bits, no parity and one stop bit.
    WritePort(SERIAL_LINE(COM1), 0x03, 1);
    // Enable FIFO and clear all buffers
    WritePort(SERIAL_FIFO(COM1), 0xC7, 1);
    // Enable IRQs, set RTS and DSR
    WritePort(SERIAL_MODEM(COM1), 0x03, 1);
}


int CheckSerial() {
    return ReadPort(SERIAL_LINE_STATUS(COM1), 1);
}

void WriteSerialChar(const char chr) {
    while(!(CheckSerial() & 0x20));
    WritePort(COM1, chr, 1);
}

void WriteSerialString(const char* str, size_t len) {
    for(size_t i = 0; i < len; i++) {
        WriteSerialChar(str[i]);
    }
}