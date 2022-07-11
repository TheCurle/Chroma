#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <editor/main.h>
#include "kernel/system/acpi/rsdt.h"
#include "kernel/system/acpi/madt.h"
#include "driver/io/apic.h"
#include "driver/io/ps2_keyboard.h"
#include "kernel/system/process/process.h"

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file is the entry point to the system.
 * It dictates the order of operations of everything the kernel actually does.
 * If a function isn't fired here, directly or indirectly, it is not run.
 */
address_space_t KernelAddressSpace;

size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;

extern "C" void mainThread();

extern "C" int Main(void) {
    KernelAddressSpace.Lock.NextTicket = 0;
    KernelAddressSpace.Lock.NowServing = 0;
    KernelAddressSpace.PML4 = nullptr;

    SerialPrintf("\r\n[ boot] Booting Chroma..\r\n");
    SerialPrintf("[ boot] Bootloader data structure at 0x%p\r\n", (size_t) &bootldr);
    SerialPrintf("[ boot] Kernel loaded at 0x%p, ends at 0x%p, is %d bytes long.\r\n", KernelAddr, KernelEnd,
                 KernelEnd - KernelAddr);
    SerialPrintf("[ boot] Framebuffer at 0x%p / 0x%p, is %dx%d, 0x%x bytes long.\r\n", bootldr.fb_ptr, (size_t) &fb,
                 bootldr.fb_width, bootldr.fb_height, bootldr.fb_size);
    SerialPrintf("[ boot] Initrd is physically at 0x%p, and is %d bytes long.\r\n", bootldr.initrd_ptr,
                 bootldr.initrd_size);
    SerialPrintf("[ boot] Initrd's header is 0x%p\r\n", FIXENDIAN32(*((volatile uint32_t*) (bootldr.initrd_ptr))));

    ParseKernelHeader(bootldr.initrd_ptr);

    SerialPrintf("[ boot] The bootloader has put the paging tables at 0x%p.\r\n", ReadControlRegister(3));
    SerialPrintf("[ boot] Removing bootloader code.\r\n");
    memset((void*) 0x600, 0, 0x6600);

    ListMemoryMap();

    InitPrint();

    PrepareCPU();
    InitMemoryManager();
    InitPaging();

    Device::APIC::driver = new Device::APIC();
    Device::PS2Keyboard::driver = new Device::PS2Keyboard();
    ProcessManager::instance = new ProcessManager();

    ACPI::RSDP::instance->Init();
    ACPI::MADT::instance->Init();

    Core::PreInit();

    Device::APIC::driver->Init();
    Device::PS2Keyboard::driver->Init();

    Core::Init();

    ProcessManager::instance->InitKernelProcess(mainThread);

    for(;;) { ProcessManager::yield(); }
}

extern "C" void mainThread() {
    SerialPrintf("Kernel thread, woooo\r\n");
    for(;;) {};
}


extern "C" void SomethingWentWrong(const char* Message) {
    SerialPrintf("Assertion failed! %s\r\n", Message);
    for(;;){}
}

extern "C" void __cxa_pure_virtual() { SomethingWentWrong("Pure Virtual Method Called"); }

extern "C" void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);
}