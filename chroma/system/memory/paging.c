#include <kernel/chroma.h>
#include <lainlib/lainlib.h>

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
 * The general plan, being that the BOOTBOOT loader has given us static addresses for all of our doodads,
 *  is to keep the core kernel where it is (FFFFFFFFFFE00000) and load in modules and libraries around it.
 * 
 * We start in the higher half, so we'll dedicate the lower half (7FFFFFFFFFFF and below) to userspace.
 * 
 * That means we have about 3 terabytes of RAM for the kernel.
 *  This will be identity mapped, always.
 * 
 * Handily, since most modern processors ignore the highest 2 bytes of a virtual address, and the kernel
 *  is mapped to 0x80000000000 and above, we can use the nomenclature:
 *      * 0x00007FFFFFFFFFFF and below is user space.
 *      * 0xFFFF800000000000 and above is kernel space.
 *  The processor will ignore the first 4 chars, and this provides a great deal of readability for the
 *   future of the kernel.
 * 
 * We'll have a kernel heap mapped into this kernel space, as well as a kernel stack (for task switching and error tracing).
 *  These will be 1GB each.
 *      We may have to increase this in the future, once Helix is fully integrated.
 *      Helix will take a lot of memory, as it is a fully featured 3D engine. We may have to implement things like
 *       texture streaming and mipmapping. Minimising RAM usage is NOT a priority for me, but it would be nice
 *        to have a minimum requirement above 32GB.
 * 
 *      // TODO: Expand Kernel Heap
 * 
 * 
 * //TODO: there are lots of calls to AllocateFrame here, those need to be separated out into AllocateZeroFrame if necessary.
 * 
 * 
 */ 

//extern size_t _kernel_text_start;
extern size_t _kernel_rodata_start;
extern size_t _kernel_data_start;

//__attribute__((aligned(4096))) static size_t Pagetable[512] = {0};

#define LAST_ENTRY 0xFF8

#define SET_ADDRESS(a,b) ((*(size_t*) (a)) = (size_t) b)

/*
 * It turns out it's useful to have macros for the standard
 * data size units.
 * 
 * Who would've thoguht?
 */

#define KiB 1 * 1024
#define MiB 1 * 1024 * KiB


#define PAGE_PRESENT 1
#define PAGE_RW      2
#define PAGE_USER    4
#define PAGE_GLOBAL  8


#define USERWRITEABLE_FLAGS(a) ((a & 0xFFFFFF00) + 0x83)

// The AbstractAllocator control struct
static allocator_t Allocator = NULL;
// The AbstractAllocator Ticketlock.
static ticketlock_t AllocatorLock = {0};

// Entries to help allocate the Kernel Stack
static list_entry_t StackFreeList;
static ticketlock_t StackLock = {0};
static void* StackPointer = (void*) KERNEL_STACK_REGION;

// A temporary itoa function for better debugging..
const char* IntToAscii(int In) {
    char* OutputBuffer = "    ";

    size_t Temp, i = 0, j = 0;

    do {
        Temp = In % 10;
        OutputBuffer[i++] = (Temp < 10) ? (Temp + '0') : (Temp + 'a' - 10);
    } while (In /= 10);

    OutputBuffer[i--] = 0;

    for(j = 0; j < i; j++, i--) {
        Temp = OutputBuffer[j];
        OutputBuffer[j] = OutputBuffer[i];
        OutputBuffer[i] = Temp;
    }

    return OutputBuffer;

}


void InitPaging() {
    StackFreeList = (list_entry_t) { &StackFreeList, &StackFreeList };

    size_t Size = AlignUpwards(AllocatorSize(), PAGE_SIZE);
    Allocator = PhysAllocateZeroMem(Size);
    Allocator = CreateAllocatorWithPool(Allocator, Size);

    SerialPrintf("[  Mem] Everything preallocated for paging.\n");

    KernelAddressSpace = (address_space_t) {
        .Lock = {0},
        .PML4 = PhysAllocateZeroMem(PAGE_SIZE)
    };

    size_t* Pagetable = KernelAddressSpace.PML4;

    //SerialPrintf("[  Mem] About to identity map the higher half.\n");
    // Identity map the higher half
    for(int i = 256; i < 512; i++) {
        Pagetable[i] = (size_t)PhysAllocateZeroMem(PAGE_SIZE);
        Pagetable[i] = (size_t)(((char*)Pagetable[i]) - DIRECT_REGION);
        Pagetable[i] |= (PAGE_PRESENT | PAGE_RW);
        //SerialPrintf("%d", i - 256);
    }

    SerialPrintf("[  Mem] Identity mapping higher half complete.\n");

    MMapEnt* TopEntry = (MMapEnt*)(((size_t) (&bootldr) + bootldr.size) - sizeof(MMapEnt));
    size_t LargestAddress = TopEntry->ptr + TopEntry->size;

    SerialPrintf("[  Mem] About to map lower memory into the Direct Region. Highest address = 0x%p\n", AlignUpwards(LargestAddress, PAGE_SIZE));
    for(size_t Address = 0; Address < AlignUpwards(LargestAddress, PAGE_SIZE); Address += PAGE_SIZE) {
        MapVirtualMemory(&KernelAddressSpace, (size_t*)(((char*)Address) + DIRECT_REGION), Address, MAP_WRITE);
    }
    SerialPrintf("[  Mem] Lower half mapping complete.\n");

    SerialPrintf("[  Mem] Mapping kernel into new memory map.\r\n");

    //TODO: Disallow execution of rodata and data, and bootldr/environment
    for(void* Address = CAST(void*, KERNEL_REGION);
            Address < CAST(void*, KERNEL_REGION + (KernelEnd - KernelAddr)); // Lower half of Kernel
            Address = CAST(void*, CAST(char*, Address) + PAGE_SIZE)) {
                MapVirtualMemory(&KernelAddressSpace, Address, (CAST(size_t, Address) - KERNEL_REGION) + KernelLocation, MAP_EXEC);
    }

    /*for(void* Address = CAST(void*, KERNEL_REGION + 0x2000);
            Address < CAST(void*, KERNEL_REGION + 0x12000); // Higher half of kernel
            Address = CAST(void*, CAST(char*, Address) + PAGE_SIZE)) {
                MapVirtualMemory(&KernelAddressSpace, Address, (CAST(size_t, Address) - KERNEL_REGION) + KERNEL_PHYSICAL_2, MAP_EXEC);
    }*/

    for(void* Address = CAST(void*, FB_REGION);
            Address < CAST(void*, 0x200000);     // TODO: Turn this into a calculation with bootldr.fb_size
            Address = CAST(void*, CAST(char*, Address) + PAGE_SIZE)) {
                MapVirtualMemory(&KernelAddressSpace, Address, (CAST(size_t, Address) - FB_REGION) + FB_PHYSICAL, MAP_WRITE);
    }

    SerialPrintf("[  Mem] Kernel mapped into pagetables. New PML4 at 0x%p\r\n", KernelAddressSpace.PML4);
    SerialPrintf("[  Mem] About to move into our own pagetables.\r\n");
    WriteControlRegister(3, (size_t) KernelAddressSpace.PML4);
    SerialPrintf("[  Mem] We survived!\r\n");
    //ASSERT(Allocator != NULL);
}

static size_t GetCachingAttribute(pagecache_t Cache) {
    switch (Cache) {
        case CACHE_WRITE_BACK: return 0;
        case CACHE_WRITE_THROUGH: return 1 << 2;
        case CACHE_NONE: return 1 << 3;
        case CACHE_WRITE_COMBINING: return 1 << 6;
    }

    return 1 << 3;
}

static bool ExpandAllocator(size_t NewSize) {
    size_t AllocSize = AlignUpwards(AllocatorPoolOverhead() + sizeof(size_t) * 5 + NewSize, PAGE_SIZE);
    void* Pool = PhysAllocateMem(AllocSize);
    return AddPoolToAllocator(Allocator, Pool, AllocSize) != NULL;
} 

static void GetPageFromTables(address_space_t* AddressSpace, size_t VirtualAddress, size_t** Page) {

    //ASSERT(Page != NULL);
    //ASSERT(AddressSpace != NULL);

    size_t* Pagetable = AddressSpace->PML4;
    for(int Level = 4; Level > 1; Level--) {
        size_t* Entry = &Pagetable[(VirtualAddress >> (12u + 9u * (Level - 1))) & 0x1FFU];

        ASSERT(*Entry & PAGE_PRESENT, "Page not present during retrieval");

        Pagetable = (size_t*)((char*)(*Entry & 0x7ffffffffffff000ull) + DIRECT_REGION);
    }

    ASSERT(Pagetable[(VirtualAddress >> 12U) & 0x1FFU] & PAGE_PRESENT, "PDPE not present during retrieval");
    *Page = &Pagetable[(VirtualAddress >> 12U) & 0x1FFU];

}

void SetAddressSpace(address_space_t* AddressSpace) {
    //ASSERT(AddressSpace != NULL);

    if((size_t)((char*)ReadControlRegister(3) + DIRECT_REGION) != (size_t) &AddressSpace->PML4) {
        WriteControlRegister(3, CAST(size_t, &AddressSpace->PML4));
    }
}

void MapVirtualMemory(address_space_t* AddressSpace, void* VirtualAddress, size_t PhysicalAddress, mapflags_t Flag) {

    //bool MapGlobally = false;
    size_t Virtual = (size_t)VirtualAddress;

    //ASSERT(AddressSpace != NULL);
    TicketAttemptLock(&AddressSpace->Lock);

    size_t Flags = PAGE_PRESENT;

    if(Flag & MAP_WRITE)
        Flags |= MAP_WRITE;
    
    if(Virtual < USER_REGION)
        Flags |= PAGE_USER;
    //TODO: Global mapping

    size_t* Pagetable = AddressSpace->PML4;
    for(int Level = 4; Level > 1; Level--) {
        size_t* Entry = &Pagetable[(Virtual >> (12u + 9u * (Level - 1))) & 0x1FFu];

        if(!(*Entry & PAGE_PRESENT)) {
            directptr_t Pointer = PhysAllocateZeroMem(PAGE_SIZE);
            *Entry = (size_t)(((char*)Pointer) - DIRECT_REGION);
        }

        *Entry |= Flags;

        Pagetable = (size_t*)(((char*)(*Entry & 0x7ffffffffffff000ull) + DIRECT_REGION));
    }

    size_t* Entry = &Pagetable[(Virtual >> 12u) & 0x1FFu];
    *Entry = Flags | PhysicalAddress;
    

    if(AddressSpace != NULL) {
        TicketUnlock(&AddressSpace->Lock);
    }

}

void UnmapVirtualMemory(address_space_t* AddressSpace, void* VirtualAddress){ 
    //ASSERT(AddressSpace != NULL);

    TicketAttemptLock(&AddressSpace->Lock);

    size_t* Entry;
    GetPageFromTables(AddressSpace, (size_t)VirtualAddress, &Entry);

    *Entry = 0;
    InvalidatePage((size_t)VirtualAddress);

    if(AddressSpace != NULL) {
        TicketUnlock(&AddressSpace->Lock);
    }

}

void CacheVirtualMemory(address_space_t* AddressSpace, void* VirtualAddress, pagecache_t Cache) {

    //ASSERT(AddressSpace != NULL);

    TicketAttemptLock(&AddressSpace->Lock);

    size_t* Entry;

    GetPageFromTables(AddressSpace, (size_t)VirtualAddress, &Entry);

    *Entry &= ~((1 << 6) | (1 << 2) | (1 << 3));
    *Entry |= GetCachingAttribute(Cache);

    InvalidatePage((size_t)VirtualAddress);

    if(AddressSpace != NULL) {
        TicketUnlock(&AddressSpace->Lock);
    }
}


void* AllocateMemory(size_t Bits) {
    TicketAttemptLock(&AllocatorLock);

    void* Result = AllocatorMalloc(Allocator, Bits);

    if(Result == NULL) {
        if(!ExpandAllocator(Bits)) {
            TicketUnlock(&AllocatorLock);
            return 0ULL;
        }

        Result = AllocatorMalloc(Allocator, Bits);
    }

    if(Result != NULL) {
        memset(Result, 0, Bits);
    }

    TicketUnlock(&AllocatorLock);
    return Result;

}

void* ReallocateMemory(void* Address, size_t NewSize) {
    TicketAttemptLock(&AllocatorLock);
    void* Result = AllocatorRealloc(Allocator, Address, NewSize);

    if(Result == NULL) {
        if(!ExpandAllocator(NewSize)) {
            TicketUnlock(&AllocatorLock);
            return 0ULL;
        }

        Result = AllocatorRealloc(Allocator, Address, NewSize);
    }

    TicketUnlock(&AllocatorLock);
    return Result;
    
}

void FreeMemory(void* Address) {
    TicketAttemptLock(&AllocatorLock);
    AllocatorFree(Allocator, Address);
    TicketUnlock(&AllocatorLock);
}

void* AllocateKernelStack() {
    void* StackAddress = NULL;
    size_t StackSize = PAGE_SIZE * 4;

    TicketAttemptLock(&StackLock);
    if(ListIsEmpty(&StackFreeList)) {
        StackAddress = StackPointer;
        StackPointer = (void*)(((char*)StackPointer) +  (4*KiB) + StackSize);

        for(size_t i = 0; i < (StackSize / PAGE_SIZE); i++) {
            directptr_t NewStack;
            NewStack = PhysAllocateZeroMem(PAGE_SIZE);
            MapVirtualMemory(&KernelAddressSpace, (void*)((size_t)StackAddress + i * PAGE_SIZE), (size_t)((char*)NewStack) - DIRECT_REGION, MAP_WRITE);
        }
    } else {
        list_entry_t* StackEntry = StackFreeList.Next;
        ListRemove(StackEntry);
        memset(StackEntry, 0, StackSize);
        StackAddress = (void*)StackEntry;
    }

    TicketUnlock(&StackLock);

    StackAddress = (void*)((size_t)StackAddress + StackSize);
    StackAddress = (void*)((size_t)StackAddress - sizeof(size_t) * 2);

    return StackAddress;
}

void FreeKernelStack(void* StackAddress) {
    TicketAttemptLock(&StackLock);
    list_entry_t* ListEntry = (list_entry_t*)(((size_t)(StackAddress) + (sizeof(size_t) * 2)) - (PAGE_SIZE * 4));
    ListAdd(&StackFreeList, ListEntry);
    TicketUnlock(&StackLock);
}
