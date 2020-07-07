#include <kernel/chroma.h>

size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;


void _start(void) {


    SerialPrintf("\r\nBooting Chroma..\r\n");
    SerialPrintf("Kernel loaded at 0x%p, ends at 0x%p, is %d bytes long.\r\n", KernelAddr, KernelEnd, KernelEnd - KernelAddr);

    ListMemoryMap();

    InitPrint();

    WriteStringWithFont("Initty Testing");
    WriteStringWithFont("Lenny");

    SetupInitialGDT();
    SetupIDT();
    InitInterrupts();

    InitMemoryManager();
    MemoryTest();

    InitPaging();


    for(;;) { }
    
}
