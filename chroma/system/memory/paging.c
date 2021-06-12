#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#define PAGE_TABLES_GET_PDPT(address) \
    (address & ((size_t) 0x1FF << 39)) >> 39
#define PAGE_TABLES_GET_PDP(address) \
    (address & ((size_t) 0x1FF << 30)) >> 30
#define PAGE_TABLES_GET_PDE(address) \
    (address & ((size_t) 0x1FF << 21)) >> 21
#define PAGE_TABLES_GET_PT(address) \
    (address & ((size_t) 0x1FF << 12)) >> 12

// The flag bit, per page, that determines whether this page is present.
#define PRESENT_BIT 1

// Default flags for a new page table. 7 = 1 | 2 | 4 = Present, writeable, accessible from userspace
#define DEFAULT_PAGE_FLAGS 7

size_t KernelLocation;

/**
 * Bootstrap the paging process.
 * Seeds the page tables, maps the kernel and framebuffer, etc.
 *
 */
void InitPaging() {

    KernelAddressSpace = (address_space_t) {
        .Lock = {0},
        .PML4 = PhysAllocateZeroMem(4096)
    };

    address_space_t BootloaderAddressSpace = (address_space_t) {
        .Lock = {0},
        .PML4 = (size_t*) ReadControlRegister(3)
    };

    size_t AddressToFind = KernelAddr + 0x2000;
    size_t BootldrAddress = 0x8000;
    KernelLocation = DecodeVirtualAddressNoDirect(&BootloaderAddressSpace, AddressToFind);
    SerialPrintf("[  Mem] Double check: Kernel physically starts at 0x%p (0x%p), ends at 0x%p.\r\n", KernelLocation, AddressToFind, KERNEL_END);

    SerialPrintf("[  Mem] Identity mapping the entirety of physical memory\r\n");

    for(size_t i = 0; i < MemorySize / PAGE_SIZE; i++) {
        size_t Addr = i * 4096;
        MapVirtualPageNoDirect(&KernelAddressSpace, Addr, Addr, DEFAULT_PAGE_FLAGS);
        MapVirtualPageNoDirect(&KernelAddressSpace, Addr, TO_DIRECT(Addr), DEFAULT_PAGE_FLAGS);
    }

    SerialPrintf("[  Mem] Mapping 0x%x bytes of bootloader structure, starting at 0x%p\r\n", bootldr.size, BootldrAddress);
    for(size_t i = BootldrAddress; i < (BootldrAddress + bootldr.size); i += PAGE_SIZE)
        MapVirtualPageNoDirect(&KernelAddressSpace, i, KERNEL_REGION + (i - BootldrAddress), 0x3);

    // This allows the code to actually run
    SerialPrintf("[  Mem] Mapping 0x%x bytes of kernel, starting at 0x%p\r\n", KERNEL_END - KERNEL_PHYSICAL, KERNEL_PHYSICAL);
    for(size_t i = KERNEL_PHYSICAL; i < KERNEL_END; i += PAGE_SIZE)
        MapVirtualPageNoDirect(&KernelAddressSpace, i, (i - KERNEL_PHYSICAL) + KERNEL_REGION + KERNEL_TEXT, 0x3);

    // TODO: The above mapping loses the ELF header.

    // This allows us to write to the screen
    SerialPrintf("[  Mem] Mapping 0x%x bytes of framebuffer, starting at 0x%p\r\n", bootldr.fb_size, FB_PHYSICAL);
    for(size_t i = FB_PHYSICAL; i < bootldr.fb_size + FB_PHYSICAL; i += PAGE_SIZE) {
        MapVirtualPageNoDirect(&KernelAddressSpace, i, i, 0x3); // FD000000 + (page)
        MapVirtualPageNoDirect(&KernelAddressSpace, i, (i - FB_PHYSICAL) + FB_REGION, 0x3); // FFFFFFFFFC000000 + (page)
    }

    // This allows us to call functions
    SerialPrintf("[  Mem] Mapping stack\r\n");
    MapVirtualPageNoDirect(&KernelAddressSpace, CORE_STACK_PHYSICAL, STACK_TOP, 0x3);

    // Make sure everything is sane
    SerialPrintf("[  Mem] Diagnostic: Querying existing page tables\r\n");

    size_t KernelAddress = DecodeVirtualAddressNoDirect(&KernelAddressSpace, AddressToFind);
    SerialPrintf("[  Mem] Diagnostic: Our pagetables put 0x%p at 0x%p + 0x%p.\r\n", AddressToFind, KernelAddress, AddressToFind & ~STACK_TOP);
    SerialPrintf("[  Mem] Diagnostic: Existing pagetables put 0x%p at 0x%p + 0x%p.\r\n", AddressToFind, KERNEL_PHYSICAL, AddressToFind & ~STACK_TOP);
    SerialPrintf("[  Mem] %s\r\n", KernelAddress == KERNEL_PHYSICAL ? "These match. Continuing." : "These do not match. Continuing with caution..");

    //if(BootloaderAddress != KernelDisoveredAddress)
        //for(;;) {}

    SerialPrintf("[  Mem] Attempting to jump into our new pagetables: 0x%p\r\n", (size_t) KernelAddressSpace.PML4);
    WriteControlRegister(3, (size_t) KernelAddressSpace.PML4 & STACK_TOP);
    SerialPrintf("[  Mem] Worked\r\n");
}


/**
 * Given the offsets in the page tables, construct a virtual address.
 *
 * Bits 0 to 16 reflect the first digit of the PDPT.
 * @param pdpt Page Directory Pointer Table - Bits 16 to 25
 * @param pdp  Page Directory Pointer -       Bits 26 to 34
 * @param pde  Page Directory Entry -         Bits 35 to 43
 * @param pt   Page Table -                   Bits 44 to 52
 * Bits 52 to 64 are the Page Offset.
 *
 * @return size_t The corresponding virtual address
 */
size_t ConstructVirtualAddress(size_t pdpt, size_t pdp, size_t pde, size_t pt) {
    return 0 | pdpt << 39 | pdp << 30 | pde << 21 | pt << 12;
}

/**
 * Given a virtual address, walk the page tables to retrieve the physical frame.
 * Note that the lowest 12 bits are CLEARED.
 *
 * The page tables are a 4 (5) dimensional array, so this function
 * walks the tables, checking that each step is present, before moving onto the next.
 * NOTE: this can be replaced with a loop.
 * WARNING: this leads to instability.
 * TODO: figure out if we can fix that?
 *
 * @param AddressSpace The address space of the process to walk
 * @param VirtualAddress The address to decode
 * @return size_t The physical frame that the virtual address encodes
 */
size_t DecodeVirtualAddress(address_space_t* AddressSpace, size_t VirtualAddress) {
    size_t PDPT = PAGE_TABLES_GET_PDPT(VirtualAddress);
    size_t PDP = PAGE_TABLES_GET_PDP(VirtualAddress);
    size_t PDE = PAGE_TABLES_GET_PDE(VirtualAddress);
    size_t PT = PAGE_TABLES_GET_PT(VirtualAddress);
    size_t* PDPT_T, *PDE_T, *PT_T;

    if(AddressSpace->PML4[PDPT] & PRESENT_BIT)
        PDPT_T = (size_t*) TO_DIRECT(AddressSpace->PML4[PDPT] & STACK_TOP);
    else
        return VirtualAddress;

    if(PDPT_T[PDP] & PRESENT_BIT)
        PDE_T = (size_t*) TO_DIRECT(PDPT_T[PDP] & STACK_TOP);
    else
        return VirtualAddress;

    if(PDE_T[PDE] & PRESENT_BIT)
        PT_T = (size_t*) TO_DIRECT(PDE_T[PDE] & STACK_TOP);
    else
        return VirtualAddress;

    return PT_T[PT] & STACK_TOP;
}

/**
 * Walk the tables, generating the structures required to map the specified Physical address to the specified Virtual Address.
 * It generates new intermediary pages as required.
 * The page table entry's flags are set to the specified PageFlags.
 *
 * @param AddressSpace The address space to map this page into
 * @param Physical The physical address to map
 * @param Virtual The virtual address to map into the physical address
 * @param PageFlags Wanted flags for the final page.
 */
void MapVirtualPage(address_space_t* AddressSpace, size_t Physical, size_t Virtual, size_t PageFlags) {
    size_t PDPT = PAGE_TABLES_GET_PDPT(Virtual);
    size_t PDP = PAGE_TABLES_GET_PDP(Virtual);
    size_t PDE = PAGE_TABLES_GET_PDE(Virtual);
    size_t PT = PAGE_TABLES_GET_PT(Virtual);
    size_t* PDPT_T, *PDE_T, *PT_T;

    // Read the top level's bits. If it's marked as present..
    if(AddressSpace->PML4[PDPT] & PRESENT_BIT)
        // Set the variable for the next level. Mask off the lower 12 bits, shift it into the "direct region".
        PDPT_T = (size_t*) TO_DIRECT(AddressSpace->PML4[PDPT] & STACK_TOP);
    else {
        // Otherwise, allocate a new page in the direct region.
        PDPT_T = (size_t*) TO_DIRECT(PhysAllocateZeroMem(4096));
        // Pull it down from the direct region, and save it as the level's page for future reads of this block.
        AddressSpace->PML4[PDPT] = FROM_DIRECT(PDPT_T) | DEFAULT_PAGE_FLAGS;
    }

    // The above repeats.
    if(PDPT_T[PDP] & PRESENT_BIT)
        PDE_T = (size_t*) TO_DIRECT(PDPT_T[PDP] & STACK_TOP);
    else {
        PDE_T = (size_t*) TO_DIRECT(PhysAllocateZeroMem(4096));
        PDPT_T[PDP] = FROM_DIRECT(PDE_T) | DEFAULT_PAGE_FLAGS;
    }

    if(PDE_T[PDE] & PRESENT_BIT)
        PT_T = (size_t*) TO_DIRECT(PDE_T[PDE] & STACK_TOP);
    else {
        PT_T = (size_t*) TO_DIRECT(PhysAllocateZeroMem(4096));
        PDE_T[PDE] = FROM_DIRECT(PT_T) | DEFAULT_PAGE_FLAGS;
    }

    // Finally, set the last page table content to the physical page + the flags we specified.
    PT_T[PT] = (size_t) (Physical | PageFlags);
}

/**
 * Given a virtual address, walk the page tables to retrieve the physical frame.
 * Note that the lowest 12 bits are CLEARED.
 *
 * This function does not touch the Direct Region, ergo making it suitable for querying
 * the initial memory maps.
 *
 * The page tables are a 4 (5) dimensional array, so this function
 * walks the tables, checking that each step is present, before moving onto the next.
 * NOTE: this can be replaced with a loop.
 * WARNING: this leads to instability.
 * TODO: figure out if we can fix that?
 *
 * @param AddressSpace The address space of the process to walk
 * @param VirtualAddress The address to decode
 * @return size_t The physical frame that the virtual address encodes
 */
size_t DecodeVirtualAddressNoDirect(address_space_t* AddressSpace, size_t VirtualAddress) {
    size_t PDPT = PAGE_TABLES_GET_PDPT(VirtualAddress);
    size_t PDP = PAGE_TABLES_GET_PDP(VirtualAddress);
    size_t PDE = PAGE_TABLES_GET_PDE(VirtualAddress);
    size_t PT = PAGE_TABLES_GET_PT(VirtualAddress);
    size_t* PDPT_T, *PDE_T, *PT_T;

    if(AddressSpace->PML4[PDPT] & PRESENT_BIT)
        PDPT_T = (size_t*) (AddressSpace->PML4[PDPT] & STACK_TOP);
    else
        return VirtualAddress;

    if(PDPT_T[PDP] & PRESENT_BIT)
        PDE_T = (size_t*) (PDPT_T[PDP] & STACK_TOP);
    else
        return VirtualAddress;

    if(PDE_T[PDE] & PRESENT_BIT)
        PT_T = (size_t*) (PDE_T[PDE] & STACK_TOP);
    else
        return VirtualAddress;

    return PT_T[PT] & STACK_TOP;
}

/**
 * Walk the tables, generating the structures required to map the specified Physical address to the specified Virtual Address.
 * It generates new intermediary pages as required.
 * The page table entry's flags are set to the specified PageFlags.
 *
 * This function does not reference the Direct region.
 * Ergo, it is suitable for initializing the first memory map the kernel needs to use.
 *
 * @param AddressSpace The address space to map this page into
 * @param Physical The physical address to map
 * @param Virtual The virtual address to map into the physical address
 * @param PageFlags Wanted flags for the final page.
 */
void MapVirtualPageNoDirect(address_space_t* AddressSpace, size_t Physical, size_t Virtual, size_t PageFlags) {
    size_t PDPT = PAGE_TABLES_GET_PDPT(Virtual);
    size_t PDP = PAGE_TABLES_GET_PDP(Virtual);
    size_t PDE = PAGE_TABLES_GET_PDE(Virtual);
    size_t PT = PAGE_TABLES_GET_PT(Virtual);
    size_t* PDPT_T, *PDE_T, *PT_T;

    // Read the top level's bits. If it's marked as present..
    if(AddressSpace->PML4[PDPT] & PRESENT_BIT)
        // Set the variable for the next level. Mask off the lower 12 bits, shift it into the "direct region".
        PDPT_T = (size_t*) (AddressSpace->PML4[PDPT] & STACK_TOP);
    else {
        // Otherwise, allocate a new page in the direct region.
        PDPT_T = (size_t*) PhysAllocateZeroMem(4096);
        // Pull it down from the direct region, and save it as the level's page for future reads of this block.
        AddressSpace->PML4[PDPT] = (size_t) PDPT_T | DEFAULT_PAGE_FLAGS;
    }

    // The above repeats.
    if(PDPT_T[PDP] & PRESENT_BIT)
        PDE_T = (size_t*) (PDPT_T[PDP] & STACK_TOP);
    else {
        PDE_T = (size_t*) PhysAllocateZeroMem(4096);
        PDPT_T[PDP] = (size_t) PDE_T | DEFAULT_PAGE_FLAGS;
    }

    if(PDE_T[PDE] & PRESENT_BIT)
        PT_T = (size_t*) (PDE_T[PDE] & STACK_TOP);
    else {
        PT_T = (size_t*) PhysAllocateZeroMem(4096);
        PDE_T[PDE] = (size_t) PT_T | DEFAULT_PAGE_FLAGS;
    }

    // Finally, set the last page table content to the physical page + the flags we specified.
    PT_T[PT] = (size_t) (Physical | PageFlags);
}

/**
 * Initialize and create a new page table.
 * The higher half of the current page table will be copied into the new one.
 * The lower 4GB will be identity mapped onto itself.
 * Therefore, it will be ready for population for a new process immediately.
 *
 * @param AddressSpace The currently loaded AddressSpace, to seed the higher half
 * @return size_t* The location of the fresh PML4
 */
size_t* CreateNewPageTable(address_space_t* AddressSpace) {
    // Allocate the first page
    size_t* NewPML4 = (size_t*) TO_DIRECT(PhysAllocateZeroMem(4096));
    address_space_t TempAddressSpace = (address_space_t) {
        .Lock = {0},
        .PML4 = NewPML4
    };

    // Initialize to zeros
    for(size_t i = 0; i < 512; i++)
        NewPML4[i] = 0;

    // Copy the current Address Space's higher half
    for(size_t i = 255; i < 512; i++)
        NewPML4[i] = AddressSpace->PML4[i];

    // Identity map the bottom two megabytes into the higher half
    for(size_t i = 0; i < 8192; i++) {
        // Get page offset
        size_t Addr = i * 4096;
        // Identity map
        MapVirtualPage(&TempAddressSpace, Addr, Addr, DEFAULT_PAGE_FLAGS);
        // Map higher half
        MapVirtualPage(&TempAddressSpace, Addr, TO_DIRECT(Addr), DEFAULT_PAGE_FLAGS);
        // TODO: Map into kernel space
    }

    // Identity map the next 4gb
    for(size_t i = 8192; i < 0x100000; i++) {
        size_t Addr = i * 4096;
        MapVirtualPage(&TempAddressSpace, Addr, Addr, DEFAULT_PAGE_FLAGS);
    }

    return NewPML4;
}
