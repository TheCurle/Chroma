#include <driver/io/ps2_mouse.h>
#include <kernel/chroma.h>
#include "kernel/video/draw.h"

Device::PS2Mouse* Device::PS2Mouse::driver;
using namespace Device;


PS2Mouse::PS2Mouse() {
    driver = this;
}

void PS2Mouse::handlePacket() {
    switch (byteRead) {
        case 0:
            byteRead++;
            packetData[0] = ReadPort(0x60, 1);
            break;
        case 1:
            byteRead++;
            packetData[1] = ReadPort(0x60, 1);
            break;
        case 2:
            packetData[2] = ReadPort(0x60, 1);
            byteRead = 0;

            xOffset = packetData[1];
            yOffset = packetData[2];

            x += xOffset;
            if (x < 0) x = 0;
            y -= yOffset;
            if (y < 0) y = 0;

            buttonStates[0] = (bool)((packetData[0] >> 2) & 1);
            buttonStates[1] = (bool)((packetData[0] >> 1) & 1);
            buttonStates[2] = (bool)((packetData[0]) & 1);
    }
}

void mouseIRQ(InterruptFrame* irq) {
    UNUSED(irq);
    uint8_t status = ReadPort(0x64, 1);
    while ((status & 0x20) == 0x20 && (status & 0x1)) {
        PS2Mouse::driver->handlePacket();
        status = ReadPort(0x64, 1);
    }
}

inline void Wait(uint8_t type) {
    size_t timeOut = 100000; //unsigned int
    if(type == 0) {
        while(timeOut--) {
            if((ReadPort(0x64, 1) & 1) == 1) {
                return;
            }
        }
        return;
    } else {
        while(timeOut--) {
            if((ReadPort(0x64, 1) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

void PS2Mouse::Init() {
    Device::RegisterDevice(this);

    SerialPrintf("[  DEV] Enabling USB Mouse..\n");

    // Enable the second PS2 port
    Wait(1);
    WritePort(0x64, 0xA8, 1);
    Wait(1);
    WritePort(0x64, 0x20, 1);
    Wait(0);
    uint8_t status = ReadPort(0x60, 1);
    Wait(1);
    WritePort(0x64, 0x60, 1);
    Wait(1);
    WritePort(0x60, status | 2, 1);

    // Enable the PS2 Mouse's "Data Reporting"
    WritePort(0x64, 0xD4, 1);
    Wait(0); ReadPort(0x60, 1);
    WritePort(0x60, 0xF4, 1);
    Wait(0); ReadPort(0x60, 1);

    uint8_t response = ReadPort(0x60, 1);
    SerialPrintf("[  DEV] Mouse responded: 0x%x. %s\n", response, (response == 0xFA ? "Mouse initialised." : "Mouse seems to be in error."));
    InstallIRQ(12, &mouseIRQ);
}