#include <driver/io/ps2_keyboard.h>
#include <kernel/chroma.h>
#include "driver/io/apic.h"

/************************
 *** Team Kitty, 2022 ***
 ***     Chroma       ***
 ***********************/

char keys[128] = {
        0, 27,
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '#',
        0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
        0,
        '*', 0,
        ' ', 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0,
        0,
        0,
        '-',
        0,
        0,
        0,
        '+',
        0,
        0,
        0,
        0,
        0,
        0, 0, 0,
        0,
        0,
        0,
};

char shiftedKeys[128] = {
        0,
        0,  // ESC
        33, // !
        64, // @
        35, // #
        36, // $
        37, // %
        94, // ^
        38, // &
        42, // *
        40, // (
        41, // )
        95, // _
        43, // +
        0,
        0,
        81, // Q
        87,
        69,
        82,
        84,
        89,
        85,
        73,
        79,
        80,  // P
        123, // {
        125, // }
        0,
        10,
        65, // A
        83,
        68,
        70,
        71,
        72,
        74,
        75,
        76,  // L
        58,  // :
        34,  // "
        126, // ~
        0,
        124, // |
        90,  // Z
        88,
        67,
        86,
        66,
        78,
        77, // M
        60, // <
        62, // >
        63, // ?
        0,
        0,
        0,
        32, // SPACE
};

Device::PS2Keyboard* Device::PS2Keyboard::driver;

using namespace Device;

void IRQRedirect(INTERRUPT_FRAME* irq) {
    UNUSED(irq);
    PS2Keyboard::driver->InterruptHandler();
}

PS2Keyboard::PS2Keyboard() {
    driver = this;
}

void PS2Keyboard::setState(bool state, uint8_t key) {
    keyStates[key] = state;

    if(state)
        lastPress = key;
    else
        lastPress = 0;

    if(callback != nullptr)
        callback[key] = state;
}

bool PS2Keyboard::isPressed(uint8_t key) const {
    return keyStates[key];
}

void PS2Keyboard::Init() {
    Device::RegisterDevice(this);
    buffer.reserve(100);

    for (size_t idx = 0; idx < 128; idx++) {
        keyStates[idx] = false;
    }

    InstallIRQ(1, IRQRedirect);
}

void PS2Keyboard::InterruptHandler() {
    uint8_t state = ReadPort(0x64, 1);

    // while keyboard has input and the buffer is not empty
    while (state & 1 && !(state & 0x20)) {
        uint8_t keycode = ReadPort(0x60, 1);
        uint8_t scancode = keycode & 0x7f;
        uint8_t keystate = !(keycode & 0x80);

        if (keystate)
            SerialPrintf("[  KEY] %c pressed.\r\n", keys[scancode]);

        state = ReadPort(0x64, 1);
        setState(keystate, scancode);
        KeyboardData data { keys[scancode], (char) scancode, (bool) keystate };
        buffer.emplace_back(data);
    }
}

void PS2Keyboard::SetCallback(uint32_t* data) {
    callback = (uint8_t*) data;

    for (size_t idx = 0; idx < 128; idx++) {
        callback[idx] = false;
    }
}
