#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include "kernel/system/acpi/rsdt.h"
#include "kernel/system/acpi/madt.h"
#include "driver/io/apic.h"
#include "driver/io/ps2_keyboard.h"

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file is the entry point to the system.
 * It dictates the order of operations of everything the kernel actually does.
 * If a function isn't fired here, directly or indirectly, it is not run.
 */

bool KernelLoaded = false;

address_space_t KernelAddressSpace;

size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;

extern "C" int Main(void) {
    KernelAddressSpace.Lock.NextTicket = 0;
    KernelAddressSpace.Lock.NowServing = 0;
    KernelAddressSpace.PML4 = nullptr;

    SerialPrintf("\r\n[ boot] Booting Chroma..\r\n");

    SerialPrintf("[ boot] Removing bootloader code.\r\n");
    memset((void*) 0x600, 0, 0x6600);

    ProcessMemoryMap();

    PrepareCPU();
    InitPIC(); // Disable the PIC controllers for (L)APIC use later

    InitMemoryManager();
    InitPaging();

    SerialPrintf("Paging complete. System initialized.\n\r");
    KernelLoaded = true;

    Device::APIC::driver = new Device::APIC();
    Device::PS2Keyboard::driver = new Device::PS2Keyboard();
    ACPI::RSDP::instance->Init(0);
    ACPI::MADT::instance->Init();

    Core::PreInit();
    Device::APIC::driver->Init();
    Core::Init();

    Device::PS2Keyboard::driver->Init();

    Device::APIC::driver->SendInterCoreInterrupt(0, 33);

    for (;;) { asm("hlt"); };
}

extern "C" void SomethingWentWrong(const char* Message) {
    SerialPrintf("Assertion failed! %s\r\n", Message);
    for(;;){}
}

extern "C" void __cxa_pure_virtual() { SomethingWentWrong("Pure Virtual Method Called"); }

extern "C" void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);
}