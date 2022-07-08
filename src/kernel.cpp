#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <driver/keyboard.h>
#include <editor/main.h>
#include "kernel/system/acpi/rsdt.h"
#include "kernel/system/acpi/madt.h"
#include "driver/io/apic.h"

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

/* This file is the entry point to the system.
 * It dictates the order of operations of everything the kernel actually does.
 * If a function isn't fired here, directly or indirectly, it is not run.
 */

static bool KernelLoaded = false;

address_space_t KernelAddressSpace;

size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;

void PrintPressedChar(KeyboardData data);
int CharPrinterCallbackID;
void TrackInternalBuffer(KeyboardData data);
int InternalBufferID;

size_t BufferLength = 0;
char* InternalBuffer;

#ifdef  __cplusplus
}
#endif

/**
 * C++ code! Scary!
 * This is a temporary measure to experiment with the Editor system.
 */

extern "C" [[noreturn]] int Main(void) {
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

    PCIEnumerate();

    InitMemoryManager();

    InitPaging();

    Printf("Paging complete. System initialized.\n\r");
    KernelLoaded = true;

    InternalBuffer = (char*) kmalloc(4096);
    SerialPrintf("[  Mem] Allocated a text buffer at 0x%p\r\n", (size_t) InternalBuffer);

    Printf("\\${FF0000}C\\${<green>}h\\${<blue>}r\\${FFFF00}o\\${FF00FF}m\\${FFFF}a ");
    Printf("\\${FFFFFF}T\\${AAAA}i\\${BBBBBB}m\\${<forgeb>}e\\${<forgey>}!\n");

    SetForegroundColor(0x00FFFFFF);

    Device::APIC::driver = new Device::APIC();

    ACPI::RSDP::instance->Init(0);
    ACPI::MADT::instance->Init();
    Device::APIC::driver->Init();

    Core::Init();

    CharPrinterCallbackID = SetupKBCallback(&PrintPressedChar);

    InternalBufferID = SetupKBCallback(&TrackInternalBuffer);

    for (;;) { }
}

extern "C" void PrintPressedChar(KeyboardData data) {
    if (!KernelLoaded) return;

    if (data.Pressed) {
        SerialPrintf("Key pressed: [\\%c (%x)]\r\n", data.Char, data.Scancode);
        Printf("%c", data.Char);
    } else {
        SerialPrintf("Key released: [\\%c]\r\n", data.Char);
    }
}

extern "C" void TrackInternalBuffer(KeyboardData data) {
    if (!data.Pressed) return;

    bool tentative = false;
    if (BufferLength > 4097) tentative = true;

    if (data.Char == '\b') {
        BufferLength--;
        tentative = false;
    }

    if (data.Scancode == 0x1C) {
        InternalBuffer[BufferLength] = '\0'; // Null-terminate to make checking easier
        if (strcmp(InternalBuffer, "editor")) {
            UninstallKBCallback(InternalBufferID);
            Editor editor;
            editor.StartEditor(CharPrinterCallbackID);
        } else if (strcmp(InternalBuffer, "zero")) {
            int returnVal = sharp_entryPoint();
            SerialPrintf("Sharp returned %d\r\n", returnVal);
        } else {
            SerialPrintf("[  Kbd] No match for %s\r\n", InternalBuffer);
        }

        memset(InternalBuffer, 0, 4098);
        BufferLength = 0;
    }

    if (!tentative && data.Scancode <= 0x2c && data.Scancode != 0x1C) {
        SerialPrintf("[  Kbd] Adding %c to the buffer.\r\n", data.Char);
        InternalBuffer[BufferLength] = data.Char;
        BufferLength++;
    }
}


extern "C" void SomethingWentWrong(const char* Message) {
    SerialPrintf("Assertion failed! %s\r\n", Message);
    for(;;){}
}

extern "C" void __cxa_pure_virtual() { SomethingWentWrong("Pure Virtual Method Called"); }

extern "C" void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);
}