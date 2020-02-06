#include <kernel/chroma.h>


void _start(void) {

    SerialPrintf("\r\nBooting Chroma..\r\n");
    InitPrint();

    SetupInitialGDT();
    SetupIDT();
    InitInterrupts();

    InitMemoryManager();
    MemoryTest();

    InitPaging();

    for(;;) { }
    
}
