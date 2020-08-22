#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/****************************************
 *  W O R K    I N     P R O G R E S S  *
 ****************************************
 *
 * This file contains functions for virtual memory management.
 * 
 * Virtual Memory Management is still a work in progress.
 * The functions here are hold-offs from old versions of the software implemented here, as well as from the EFI version of Chroma, called Sync.
 * 
 * There, these functions worked, but here, under BIOS, it's a lot more difficult.
 * It will take some time to get these functions working.
 * 
 */ 


//__attribute__((aligned(4096))) static size_t Pagetable[512] = {0};

#define LAST_ENTRY 0xFF8

#define SET_ADDRESS(a,b) ((*(size_t*) (a)) = (size_t) b)

#define KiB 1 * 1024
#define MiB 1 * 1024 * KiB

#define USERWRITEABLE_FLAGS(a) ((a & 0xFFFFFF00) + 0x83)

#define PAGE_PRESENT 1
#define PAGE_RW      2


void InitPaging() {

    size_t* PML4 = (size_t*) 0xFFA000; // Layer 4
    size_t* PDPE_RAM = (size_t*) 0xFFE000; // Layer 3, contains map for the first 4GB of RAM
    size_t* PDE_RAM = (size_t*) 0xFFF000;

    size_t* PDPE_KERNEL = (size_t*) 0xFFB000; // Layer 3, contains map for the Kernel and everything it needs to run.
    size_t* PDE_KERNEL_FB = (size_t*) 0xFFC000; // Layer 2, contains map for the linear framebuffer.

    size_t* PT_KERNEL = (size_t*) 0xFFD000; // Layer 1, the page table for the kernel itself.

    size_t fb_ptr = (size_t) &fb;

    SET_ADDRESS(PML4, PDPE_RAM); // 3rd Layer entry for RAM
    SET_ADDRESS(PML4 + LAST_ENTRY, PDPE_KERNEL); // 3rd Layer entry for Kernel

    SET_ADDRESS(PDPE_KERNEL + LAST_ENTRY, PDE_KERNEL_FB); // 2nd Layer entry for the framebuffer

    // Set the 480th entry (PDE_KERNEL_FB + (480 * 8))
    // To the framebuffer + flags
    SET_ADDRESS(PDE_KERNEL_FB + 3840, USERWRITEABLE_FLAGS(fb_ptr)); 

    // In 4 byte increments, we're gonna map 3840 (the framebuffer)
    // Up to (4096 - 8) in the PDE_KERNEL_FB with 2MB paging.
    size_t MappingIterations = 1;
    for(size_t i = 3844; i < 4088; i += 4) {
        SET_ADDRESS(PDE_KERNEL_FB + i, USERWRITEABLE_FLAGS(fb_ptr) + (MappingIterations * (2 * MiB)));
        MappingIterations++;
    }

    // Now we map the last entry of PDE_KERNEL_FB to our Page Table
    SET_ADDRESS(PDE_KERNEL_FB + LAST_ENTRY, PT_KERNEL);

    // Mapping the kernel into the page tables....

    SET_ADDRESS(PT_KERNEL, 0xFF8001); // bootldr, bootinfo
    SET_ADDRESS(PT_KERNEL + 8, 0xFF9001); // environment

    // Map the kernel itself
    SET_ADDRESS(PT_KERNEL + 16, KernelAddr + 1);

    // Iterate through the pages, identity mapping each one
    MappingIterations = 1;
    size_t MappingOffset = 0x14;
    for(size_t i = 0; i < ((KernelEnd - KernelAddr) >> 12); i++) {
        // Page Table + (0x10 increasing by 0x04 each time) = x * 4KiB
        SET_ADDRESS(PT_KERNEL + MappingOffset, (MappingIterations * (4 * KiB)));
        MappingOffset += 4;
        MappingIterations++;
    }

    // Now we need to map the core stacks. Top-down, from 0xDFF8
    // There's always at least one core, so we do that one fixed.
    // TODO: Account for 0-core CPUs
    SET_ADDRESS(PT_KERNEL + LAST_ENTRY, 0xF14003);
    MappingIterations = 1;
    // For every core:
    for(size_t i = 0; i < (bootldr.numcores + 3U) >> 2; i++) {
        // PT_KERNEL[512 - (iterations + 1)] = 0x14003 + (iterations * page-width) 
        SET_ADDRESS(PT_KERNEL + LAST_ENTRY - (MappingIterations * 8), 0xF14003 + (4096 * MappingIterations));
        MappingIterations++;
    }

    SET_ADDRESS(PDPE_RAM, PDE_RAM + PAGE_PRESENT + PAGE_RW);
    SET_ADDRESS(PDPE_RAM + 8, 0xF10000 + PAGE_PRESENT + PAGE_RW);
    SET_ADDRESS(PDPE_RAM + 16, 0xF11000 + PAGE_PRESENT + PAGE_RW);
    SET_ADDRESS(PDPE_RAM + 24, 0xF12000 + PAGE_PRESENT + PAGE_RW);
    
    // Identity map 4GB of ram
    // Each page table can only hold 512 entries, but we
    // just set up 4 of them - overflowing PDE_RAM (0xF000)
    // will take us into 0x10000, into 0x11000, into 0x120000.
    for(size_t i = 0; i < 512 * 4/*GB*/; i++) {
        // add PDE_RAM, 4
        // mov eax, 0x83
        // add eax, 2*1024*1024
        SET_ADDRESS(PDE_RAM + (i * 4), USERWRITEABLE_FLAGS(i * (2 * MiB)));
    }

    // Map first 2MB of memory
    SET_ADDRESS(PDE_RAM, 0xF13000 + PAGE_PRESENT + PAGE_RW);

    for(size_t i = 0; i < 512; i++) {
        SET_ADDRESS(0xF13000 + i * 4, i * (4 * KiB) + PAGE_PRESENT + PAGE_RW);
    }

    // 0xA000 should now contain our memory map.

}


void InitPagingOldImpl() {

    // Disable paging so that we can work with the pagetable
    //size_t registerTemp = ReadControlRegister(0);
    //UNSET_PGBIT(registerTemp);
    //WriteControlRegister(0, registerTemp);

    // Clear space for our pagetable
    size_t PagetableDest = 0x1000;
    memset((char*)PagetableDest, 0, 4096);

    // Start setting pagetable indexes
    *((size_t*)PagetableDest) = 0x2003; // PDP at 0x2000, present & r/w
    *((size_t*)PagetableDest + 0x1000) = 0x3003; // PDT at 0x3000, present & r/w
    *((size_t*)PagetableDest + 0x2000) = 0x4003; // PT at 0x4000, present & r/w

    size_t value = 0x3;
    size_t offset = 8;
    for(size_t i = 0; i < 512; i++) { // 512 iterations (entries into the page table)
        *((size_t*) PagetableDest + offset) = value; // We're setting 512 bytes with x003
        // (identity mapping the first 4 megabytes of memory)
        // (mapping the page table to itself)
        value += 4096; // Point to start of next page
        offset += 8; // + 8 bytes (next entry in list)
    }

    // Enable PAE paging
    size_t reg = ReadControlRegister(4);
    SET_PAEBIT(reg);
    WriteControlRegister(4, reg);

    WriteControlRegister(3, PagetableDest);
    
}


/*    size_t registerTemp = ReadControlRegister(4);
    if(registerTemp & (1 << 7)) {
        TOGGLE_PGEBIT(registerTemp);
        WriteControlRegister(4, registerTemp);
    }

    if(registerTemp & (1 << 7))
        WriteControlRegister(4, registerTemp ^ (1 << 7));

    size_t CPUIDReturn;
    asm volatile("cpuid" : "=d" (CPUIDReturn) : "a" (0x80000001) : "%rbx", "%rcx");

    if(CPUIDReturn & (1 << 26)) {
        SerialPrintf("System supports 1GB pages.\r\n");
        
        if(registerTemp & (1 << 12)) {
            SerialPrintf("PML5 paging available - using that instead.\r\n");

            if(MemorySize > (1ULL << 57))
                SerialPrintf("System has over 128Petabytes of RAM. Please consider upgrading the OS on your supercomputer.\r\n");                
            
            size_t MaxPML5 = 1;
            size_t MaxPML4 = 1;
            size_t MaxPDP = 512;

            size_t LastPML4Entry = 512;
            size_t LastPDPEntry = 512;

            size_t MemorySearchDepth = MemorySize;

            while(MemorySearchDepth > (256ULL << 30)) {
                MaxPML5++; 
                MemorySearchDepth -= (256ULL << 30);
            }

            if(MaxPML5 > 512)
                MaxPML5 = 512;
            
            if(MemorySearchDepth) {
                LastPDPEntry = ( (MemorySearchDepth + ((1 << 30) - 1)) & (~0ULL << 30)) >> 30;

                if(MaxPML5 > 512)
                    MaxPML5 = 512;
                
            }

            size_t PML4Size = PAGETABLE_SIZE * MaxPML5;
            size_t PDPSize = PML4Size * MaxPML4;

            size_t PML4Base = AllocatePagetable(PML4Size + PDPSize);
            size_t PDPBase = PML4Base + PML4Size;

            for(size_t PML5Entry = 0; PML5Entry < MaxPML5; PML5Entry++) {
                Pagetable[PML5Entry] = PML4Base + (PML5Entry << 12);

                if(PML5Entry == (MaxPML5 - 1))
                    MaxPML4 = LastPML4Entry;

                for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++) {

                    ((size_t*) Pagetable[PML5Entry])[PML4Entry] = PDPBase + (((PML5Entry << 9) + PML5Entry) << 12);

                    if( (PML5Entry == (MaxPML5 - 1)) && (PML4Entry == (MaxPML4 -1)) )
                        MaxPDP = LastPDPEntry;
                    
                    for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                        ((size_t* ) ((size_t* ) Pagetable[PML5Entry])[PML4Entry])[PDPEntry] = ( ((PML5Entry << 18) + (PML4Entry << 9) + PDPEntry) << 30) | (0x83);
                    }

                    ((size_t* ) Pagetable[PML5Entry])[PML4Entry] |= 0x3;
                }

                Pagetable[PML5Entry] |= 0x3;
            }
        } else {
            SerialPrintf("PML4 available - using that instead.\r\n");
            size_t MemorySearchDepth = MemorySize;

            if(MemorySearchDepth > (1ULL << 48))
                SerialPrintf("RAM limited to 256TB.\r\n");
            
            size_t MaxPML4 = 1;
            size_t MaxPDP = 512;

            size_t LastPDPEntry = 512;

            while(MemorySearchDepth > (512ULL << 30)) {
                MaxPML4++;
                MemorySearchDepth -= (512ULL << 30);
            }

            if(MaxPML4 > 512)
                MaxPML4 = 512;

            if(MemorySearchDepth) {
                LastPDPEntry = ( (MemorySearchDepth + ((1 << 30) - 1)) & (~0ULL << 30)) >> 30;

                if(LastPDPEntry > 512)
                    LastPDPEntry = 512;
            }

            size_t PDPSize = PAGETABLE_SIZE * MaxPML4;
            size_t PDPBase = AllocatePagetable(PDPSize);

            for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++) {
                Pagetable[PML4Entry] = PDPBase + (PML4Entry << 12);

                if(PML4Entry == (MaxPML4 - 1)) {
                    MaxPDP = LastPDPEntry;
                }

                for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                    ((size_t* ) Pagetable[PML4Entry])[PDPEntry] = (((PML4Entry << 9) + PDPEntry) << 30) | 0x83;
                }

                Pagetable[PML4Entry] |=  0x3;
            }
        }
    } else {
        SerialPrintf("System does not support 1GB pages - using 2MiB paging instead.\r\n");

        size_t MemorySearchDepth = MemorySize;

        if(MemorySearchDepth > (1ULL << 48)) {
            SerialPrintf("Usable RAM is limited to 256TB, and the page table alone will use 1GB of space in memory.\r\n");
        }

        size_t MaxPML4 = 1, MaxPDP = 512, MaxPD = 512, LastPDPEntry = 1;

        while(MemorySearchDepth > (512ULL << 30)) {
            MaxPML4++;
            MemorySearchDepth -= (512ULL << 30);
        }

        if(MaxPML4 > 512)
            MaxPML4 = 512;

        if(MemorySearchDepth) {
            LastPDPEntry = ((MemorySearchDepth + ((1 << 30) - 1)) & (~0ULL << 30)) >> 30;

            if(LastPDPEntry > 512)
                LastPDPEntry = 512;
        }

        size_t PDPSize = PAGETABLE_SIZE * MaxPML4;
        size_t PDSize = PDPSize * MaxPDP;

        size_t PDPBase = AllocatePagetable(PDPSize + PDSize);
        size_t PDBase = PDPBase + PDSize;

        for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++) {
            Pagetable[PML4Entry] = PDBase + (PML4Entry << 12);

            if(PML4Entry == (MaxPML4 - 1)) {
                MaxPDP = LastPDPEntry;
            }

            for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                ( (size_t* ) Pagetable[PML4Entry])[PDPEntry] = PDBase + (((PML4Entry << 9) + PDPEntry) << 12);

                for(size_t PDEntry = 0; PDEntry < MaxPD; PDEntry++) {
                    ( (size_t* )  ((size_t*) Pagetable[PML4Entry])[PDPEntry])[PDEntry] = (( (PML4Entry << 18) + (PDPEntry << 9) + PDPEntry) << 21) | 0x83;
                }

                ( (size_t* ) Pagetable[PML4Entry])[PDPEntry] |= 0x3;
            }

            Pagetable[PML4Entry] |= 0x3;
        }
    }

    WriteControlRegister(3, Pagetable);

    registerTemp = ReadControlRegister(4);
    if(!(registerTemp & (1 << 7))) {
        TOGGLE_PGEBIT(registerTemp);
        WriteControlRegister(4, registerTemp);
    }*/