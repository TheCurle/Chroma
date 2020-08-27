#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file is the entry point to the system.
 * It dictates the order of operations of everything the kernel actually does.
 * If a function isn't fired here, directly or indirectly, it is not run.
 */



size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;


int Main(void) {


    SerialPrintf("\r\nBooting Chroma..\r\n");
    SerialPrintf("Kernel loaded at 0x%p, ends at 0x%p, is %d bytes long.\r\n", KernelAddr, KernelEnd, KernelEnd - KernelAddr);

    SerialPrintf("The bootloader has put the paging tables at 0x%p.\r\n", ReadControlRegister(3));

    TraversePageTables();

    ListMemoryMap();

    InitPrint();

    WriteStringWithFont("Initty Testing");

    SetupInitialGDT();
    SetupIDT();
    InitInterrupts();

    PCIEnumerate();

    InitMemoryManager();
    MemoryTest();

    InitPaging();


    for(;;) { }

    return 0;
    
}


void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);

}