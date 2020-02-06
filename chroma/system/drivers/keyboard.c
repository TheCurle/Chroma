#include <kernel/chroma.h>

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



void UpdateKeyboard(uint8_t msg) {

    InputBuffer[0] = msg;

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



    if(msg & 0x80) {

    } else {
        SerialPrintf("Key pressed %c\r\n", keys[msg]);
        WriteChar(keys[msg]);
    }

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