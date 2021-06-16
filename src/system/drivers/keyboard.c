#include <kernel/chroma.h>
#include <kernel/system/driver/keyboard.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains (mostly unused) implementations of a full PS/2 keyboard driver.
 *
 * It provides provisions for full 2-way communication, as well as auxiliary key commands.
 * //TODO: Media keys?
 *
 * Once this driver is to a workable state, I would like to start adding a proper keyboard buffer,
 *   which will integrate with a window system.
 *
 */

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

KeyboardCallback KeyboardCallbacks[16] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static int CurrentCallback = 0;

int SetupKBCallback(void (*Handler)(KeyboardData Frame)) {
    KeyboardCallbacks[CurrentCallback++] = Handler;
    return CurrentCallback;
}

void UninstallKBCallback(int Number) {
	KeyboardCallbacks[Number] = NULL; // 0 is used in the common check to make sure that the function is callable.
	// This removes this callback from that check, ergo the function will no longer be called.
}

void KbdEcho() {
    if(!KbdFlags.EchoCount) {
        if(!KbdFlags.Echo) {
            Send8042(0xEE);
        }
    } else {
        KbdFlags.EchoCount = 0;
        KbdFlags.Echo = 0;
    }
}


void UpdateKeyboard(uint8_t msg) {

    switch(msg) {
        case 0x0:
            KbdFlags.Error = 1;
            //ResendBuffer();
            break;
        case 0xAA:
            KbdFlags.SelfTest = 1;
            break;
        case 0xEE:
            KbdFlags.Echo = 0;
            KbdFlags.EchoCount = 2;
            KbdEcho();
            break;
        case 0xFA:
            KbdFlags.ACK = 1;
            //ProgressBuffer();
            break;
        case 0xFC:
        case 0xFD:
            KbdFlags.SelfTest = 0;
            KbdFlags.Error = 1;
            //RestartKbd();
            break;
        case 0xFE:
            //ResendBuffer();
            break;
        case 0xFF:
            KbdFlags.Error = 1;
            //ResendBuffer();
            break;
        default:
            break;
    }

    KeyboardData data = (KeyboardData) {
        .Scancode = msg,
        .Pressed = msg > 0x80 && msg < 0xD8 ? false : true,
        .Char = msg > 0x80 && msg < 0xD8 ? keys[msg - 0x80] : keys[msg]
    };

    void (*Handler)(KeyboardData data);

    for(size_t handlerNum = 0; handlerNum < 16; handlerNum++) {
	    Handler = KeyboardCallbacks[handlerNum];
	    if(Handler) {
	    	Handler(data);
	    }
    }
}

void Send8042(size_t info) {
    for(size_t i = 0; i < 8; i++) {
        unsigned char chr = (unsigned char) info;
        if(chr != 0) {
            WritePort(0x60, chr, 1);
            WaitFor8042();
        }
    }
}

void WaitFor8042() {

	bool full = true;
	while(full) {
		full = ReadPort(0x64, 1) & 1;
	}
}