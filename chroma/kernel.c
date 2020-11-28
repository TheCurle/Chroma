#include <kernel/chroma.h>
//#include <kernel/system/screen.h>

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

address_space_t KernelAddressSpace;

int Main(void) {
    KernelAddressSpace = (address_space_t) {0};
    KernelLocation = 0x112600;

    SerialPrintf("\r\n[ boot] Booting Chroma..\r\n");
    SerialPrintf("[ boot] Kernel loaded at 0x%p, ends at 0x%p, is %d bytes long.\r\n", KernelAddr, KernelEnd, KernelEnd - KernelAddr);
    SerialPrintf("[ boot] Initrd is physically at 0x%p, and is %d bytes long.\r\n", bootldr.initrd_ptr, bootldr.initrd_size);

    SerialPrintf("[ boot] Searching for kernel... Constants start at 0x%p\r\n", &_kernel_text_start);
    // We stop at the constants in the kernel, otherwise we'll read the constant ELF64MAGIC which is stored inside the kernel...
    
    size_t headerLoc = 0;
    for(size_t i = KernelAddr; i < KernelEnd; i++) {
        if(i < (size_t) (&_kernel_text_start) - KernelAddr) {
            if(*((volatile uint32_t*)(i)) == ELF64MAGIC) {
                SerialPrintf("[ boot] Matched kernel header at 0x%p.\r\n", i);
            }
        }
    }

    int flag = 0;

    if(headerLoc) {
        ELF64Header_t* PotentialKernelHeader = (ELF64Header_t*) &headerLoc;
        SerialPrintf(
            "[ boot] Considering ELF with:\r\n\tBitness %d\r\n\tEntry point 0x%p\r\n\tFile type %s : %d\r\n\tArchitecture %s : %d\r\n",
            PotentialKernelHeader->Class == 2 ? 64 : 32, PotentialKernelHeader->EntryPoint, PotentialKernelHeader->Type == 0x02 ? "EXECUTABLE" : "OTHER", PotentialKernelHeader->Type, PotentialKernelHeader->TargetArchitecture == 0x3E ? "AMD64" : "OTHER", PotentialKernelHeader->TargetArchitecture);
        if(PotentialKernelHeader->EntryPoint == KernelAddr) {
            SerialPrintf("[ boot] Header at 0x%p matches kernel header.\r\n", headerLoc);
            flag = 1;
        }

        if(!flag) {

            for(size_t i = 0; i < 8; i++) {
                SerialPrintf("[ boot] Header dump part %d: 0x%x\r\n", i, *((volatile uint32_t*)(headerLoc + i)));
            }
        }
    }

    if(!flag) {
        SerialPrintf("[ boot] Unable to find kernel in memory. Fatal error.\r\n");
        //for(;;) {}
    }

    SerialPrintf("[ boot] The bootloader has put the paging tables at 0x%p.\r\n", ReadControlRegister(3));

    //TraversePageTables();

    ListMemoryMap();

    InitPrint();

    WriteStringWithFont("Initty Testing");

    PrepareCPU();

    PCIEnumerate();

    InitMemoryManager();

    //DrawSplash();

    InitPaging();


    for(;;) { }

    return 0;
    
}

void SomethingWentWrong(const char* Message) {
    SerialPrintf("Assertion failed! %s\r\n", Message);
    //for(;;){}
}

void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);

}