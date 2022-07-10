#include <kernel/chroma.h>
/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/*
 * This file provides utility functions for parsing ELF headers.
 *  This exists so that the kernel can find itself for remapping,
 *   but I may end up using ELF as the kernel's executable format.
 *  Writing an ELF loader is on the to-do list, after all.
 ! This needs to be cleaned up.
 ! This creates a mess of numbers on the print
 ! This is hacky and hardcoded as heck and needs to be fixed
*/
extern size_t KernelLocation;

int ParseKernelHeader(size_t InitrdPtr) {
    int flag = 0;

    SerialPrintf("[ boot] Searching for kernel... Constants start at 0x%p / 0x%p\r\n",
                 ((size_t) (&_kernel_text_start) - KernelAddr) + InitrdPtr, (size_t) (&_kernel_text_start));
    // We stop at the constants in the kernel, otherwise we'll read the constant ELF64MAGIC which is stored inside the kernel...

    size_t headerLoc = 0;
    for (size_t i = InitrdPtr; i < ((size_t) (&_kernel_text_start) - KernelAddr) + InitrdPtr; i++) {
        if (*((volatile uint32_t*) (i)) == ELF64MAGIC) {
            SerialPrintf("[ boot] Matched kernel header at 0x%p.\r\n", i);
            headerLoc = i;
        }

        if (FIXENDIAN32(*((volatile uint32_t*) (i))) == ELF64MAGIC) {
            SerialPrintf("[ boot] Matched little-endian kernel header at 0x%p.\r\n", i);
            headerLoc = i;
        }
    }

    if (headerLoc) {
        /* For whatever reason, reading a size_t here fails. The max that seems to work is uint16_t, so we read in the 
         *  64 bit address by constructing it from 4 individual reads.
         * Note that these 4 reads are little endian, so we need to flip them around individually
         */
        uint16_t EntryPoint0 = FIXENDIAN16(*((volatile uint16_t*) (headerLoc + ELFENTRY_OFF)));
        uint16_t EntryPoint1 = FIXENDIAN16(*((volatile uint16_t*) (headerLoc + ELFENTRY_OFF + 2)));
        uint16_t EntryPoint2 = FIXENDIAN16(*((volatile uint16_t*) (headerLoc + ELFENTRY_OFF + 4)));
        uint16_t EntryPoint3 = FIXENDIAN16(*((volatile uint16_t*) (headerLoc + ELFENTRY_OFF + 6)));
        size_t EntryPoint = ((size_t) EntryPoint0 << 48) | ((size_t) EntryPoint1 << 32) | ((size_t) EntryPoint2 << 16) |
                            ((size_t) EntryPoint3);

        /* At this point, EntryPoint is a little-endian 64 bit integer. That means we have to fix its endianness in order to read it */
        SerialPrintf("[ boot] Fixing entry point from 0x%p to 0x%p\r\n", EntryPoint, FIXENDIAN64(EntryPoint));
        EntryPoint = FIXENDIAN64(EntryPoint);

        /* Now we can continue as normal */
        uint8_t HeaderClass = *((volatile uint8_t*) (headerLoc + ELF_IDENT_CLASS_OFF));
        uint16_t ExecutableType = (uint16_t) *((volatile uint8_t*) (headerLoc + ELFTYPE_OFF));
        uint16_t MachineType = (uint16_t) *((volatile uint8_t*) (headerLoc + ELFMACHINE_OFF));


        SerialPrintf(
                "[ boot] ELF header at 0x%p.\r\n\tConsidering ELF with:\r\n\tBitness %d: %d\r\n\tEntry point 0x%p\r\n\tFile type %s : 0x%x\r\n\tArchitecture %s : 0x%x\r\n",
                headerLoc,
                HeaderClass == 2 ? 64 : 32,
                HeaderClass,
                EntryPoint,
                ExecutableType == FIXENDIAN16(0x0200) ? "EXECUTABLE" : "OTHER",
                FIXENDIAN16(ExecutableType),
                MachineType == FIXENDIAN16(0x3E00) ? "AMD64" : "OTHER",
                FIXENDIAN16(MachineType));


        if (EntryPoint == (size_t) (&_kernel_text_start)) {
            SerialPrintf("[ boot] Header at 0x%p matches kernel header.\r\n", headerLoc);
            flag = 1;
            // At this point, we've found the right ELF64 executable!
            // Great, now we can map it into the proper place
            KernelLocation = headerLoc;
        }

        if (!flag) {

            for (char i = 0; i < 64; i++) {
                SerialPrintf("[ boot] Header dump part %x: 0x%x\r\n", i, *((volatile uint8_t*) (headerLoc + i)));
            }
        }
    }

    return flag;

}
