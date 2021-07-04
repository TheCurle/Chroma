#include <kernel/chroma.h>
#include <kernel/video/draw.h>
#include <kernel/system/driver/keyboard.h>
#include <editor/main.h>

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

size_t KernelAddr = (size_t) &LoadAddr;
size_t KernelEnd = (size_t) &end;

address_space_t KernelAddressSpace;

void PrintPressedChar(KeyboardData data);
int CharPrinterCallbackID;
void TrackInternalBuffer(KeyboardData data);
int InternalBufferID;

size_t BufferLength = 0;
char* InternalBuffer;

int Main(void) {
    KernelAddressSpace = (address_space_t) {0};

    SerialPrintf("\r\n[ boot] Booting Chroma..\r\n");
    SerialPrintf("[ boot] Bootloader data structure at 0x%p\r\n", (size_t) &bootldr);
    SerialPrintf("[ boot] Kernel loaded at 0x%p, ends at 0x%p, is %d bytes long.\r\n", KernelAddr, KernelEnd, KernelEnd - KernelAddr);
    SerialPrintf("[ boot] Framebuffer at 0x%p / 0x%p, is %dx%d, 0x%x bytes long.\r\n", bootldr.fb_ptr, (size_t) &fb, bootldr.fb_width, bootldr.fb_height, bootldr.fb_size);
    SerialPrintf("[ boot] Initrd is physically at 0x%p, and is %d bytes long.\r\n", bootldr.initrd_ptr, bootldr.initrd_size);
    SerialPrintf("[ boot] Initrd's header is 0x%p\r\n", FIXENDIAN32(*((volatile uint32_t*)(bootldr.initrd_ptr))));

    ParseKernelHeader(bootldr.initrd_ptr);

    SerialPrintf("[ boot] The bootloader has put the paging tables at 0x%p.\r\n", ReadControlRegister(3));

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

    // TODO: WriteString should accept escape color codes!
    SetForegroundColor(0x00FF0000);
    WriteChar('C');
    SetForegroundColor(0x0000FF00);
    WriteChar('h');
    SetForegroundColor(0x000000FF);
    WriteChar('r');
    SetForegroundColor(0x00FFFF00);
    WriteChar('o');
    SetForegroundColor(0x00FF00FF);
    WriteChar('m');
    SetForegroundColor(0x0000FFFF);
    WriteChar('a');
    WriteChar(' ');
    SetForegroundColor(0x00FFFFFF);
    WriteChar('T');
    SetForegroundColor(0x0000AAAA);
    WriteChar('i');
    SetForegroundColor(0x00BBBBBB);
    WriteChar('m');
    SetForegroundColor(0x001E2D42);
    WriteChar('e');
    SetForegroundColor(0x00E0A969);
    WriteChar('!');
    WriteChar('\n');

    SetForegroundColor(0x00FFFFFF);

    CharPrinterCallbackID = SetupKBCallback(&PrintPressedChar);

    InternalBufferID = SetupKBCallback(&TrackInternalBuffer);

    for (;;) {}

    return 0;
}

void PrintPressedChar(KeyboardData data) {
    if(!KernelLoaded) return;

    if(data.Pressed) {
        SerialPrintf("Key pressed: [\\%c (%x)]\r\n", data.Char, data.Scancode);
        Printf("%c", data.Char);
    } else {
        SerialPrintf("Key released: [\\%c]\r\n", data.Char);
    }
}

void TrackInternalBuffer(KeyboardData data) {
    if(!data.Pressed) return;

    bool tentative = false;
    if(BufferLength > 4097) tentative = true;

    if(data.Char == '\b') {
        BufferLength--;
        tentative = false;
    }

    if(data.Scancode == 0x1C) {
        InternalBuffer[BufferLength] = '\0'; // Null-terminate to make checking easier
        if(strcmp(InternalBuffer, "editor")) {
            UninstallKBCallback(InternalBufferID);
            StartEditor(CharPrinterCallbackID);
        } else if(strcmp(InternalBuffer, "zero")) {
            int returnVal = sharp_entryPoint();
            SerialPrintf("Sharp returned %d\r\n", returnVal);
        } else {
            SerialPrintf("[  Kbd] No match for %s\r\n", InternalBuffer);
        }

        memset(InternalBuffer, 0, 4098);
        BufferLength = 0;
    }

    if(!tentative && data.Scancode <= 0x2c && data.Scancode != 0x1C) {
        SerialPrintf("[  Kbd] Adding %c to the buffer.\r\n", data.Char);
        InternalBuffer[BufferLength] = data.Char;
        BufferLength++;
    }
}

void SomethingWentWrong(const char* Message) {
    SerialPrintf("Assertion failed! %s\r\n", Message);
    //for(;;){}
}

void Exit(int ExitCode) {
    SerialPrintf("Kernel stopped with code %x\r\n", ExitCode);

}

#ifdef  __cplusplus
}
#endif