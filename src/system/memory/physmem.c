#include <kernel/chroma.h>
#include <lainlib/lainlib.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains functions for physical memory management.
 *
 * Physical Memory Management is performed with Buddy List allocators, which are one of the most performant systems available.
 * They tend to be able to allocate physical pages with O(1) efficiency.
 * 
 * The implementation here is bespoke, and in need of documentation.
 * 
 * TODO: Document this mess.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MIN_ORDER 3
#define PEEK(type, address) (*((volatile type*)(address)))

uint8_t* MemoryStart;
size_t MemoryBuckets;

size_t MemoryPages = 0;
size_t FreeMemorySize = 0;
size_t FullMemorySize = 0;

static buddy_t LowBuddy = {
        .MaxOrder = 32,
        .Base = (directptr_t) DIRECT_REGION,
        .List = (directptr_t[32 - MIN_ORDER]) {0},
        .Lock = {.NowServing = 0, .NextTicket = 0},
};

static buddy_t HighBuddy = {
        .MaxOrder = 64,
        .Base = 0,
        .List = (directptr_t[64 - MIN_ORDER]) {0},
        .Lock = {.NowServing = 0, .NextTicket = 0},
};

static size_t MemoryLength;

static bool CheckBuddies(buddy_t* Buddy, directptr_t InputA, directptr_t InputB, size_t Size) {
    size_t LowerBuddy = MIN(CAST(size_t, InputA), CAST(size_t, InputB)) - (size_t) Buddy->Base;
    size_t HigherBuddy = MAX(CAST(size_t, InputA), CAST(size_t, InputB)) - (size_t) Buddy->Base;

    return (LowerBuddy ^ Size) == HigherBuddy;
}

static void AddToBuddyList(buddy_t* Buddy, directptr_t Address, size_t Order, bool NewEntry) {
    directptr_t ListHead = Buddy->List[Order - MIN_ORDER];

    //SerialPrintf("Adding new entry to buddy: Address 0x%p with order %d, New Entry is %s\r\n", Address, Order, NewEntry ? "true" : "false");

    size_t Size = 1ull << Order;

    TicketAttemptLock(&Buddy->Lock);

    if (!NewEntry && ListHead != 0) {
        directptr_t ListPrevious = 0;

        while (true) {
            if (CheckBuddies(Buddy, ListHead, Address, Size)) {
                if (ListPrevious != 0) {
                    PEEK(directptr_t, ListPrevious) = PEEK(directptr_t, ListHead);
                } else
                    Buddy->List[Order - MIN_ORDER] = PEEK(directptr_t, ListHead);

                AddToBuddyList(Buddy, MIN(ListHead, Address), Order + 1, false);
                break;
            }

            if (PEEK(directptr_t, ListHead) == 0) {
                PEEK(directptr_t, ListHead) = Address;
                break;
            }

            ListPrevious = ListHead;
            ListHead = PEEK(directptr_t, ListHead);
        }
    } else {
        *((size_t*) (Address)) = (size_t) ListHead;
        Buddy->List[Order - MIN_ORDER] = Address;
    }

    TicketUnlock(&Buddy->Lock);
}

static void AddRangeToBuddy(buddy_t* Buddy, directptr_t Base, size_t Size) {
    while (Size > (1ull << MIN_ORDER)) {
        //SerialPrintf("New iteration. Current Size: 0x%x\r\n\t", Size);
        for (int Order = Buddy->MaxOrder - 1; Order >= MIN_ORDER; Order--) {
            //SerialPrintf("New Loop. Current Order: %d\r\n\t", Order);
            if (Size >= (1ull << Order)) {
                //SerialPrintf("\tNew loop check passed.\r\n\t");
                AddToBuddyList(Buddy, Base, Order, true);
                //SerialPrintf("\tEntry added to current buddy. Moving onto memory operations..\r\n\t");
                Base = (void*) ((((char*) Base) + (1ull << Order)));
                Size -= 1ull << Order;
                //SerialPrintf("\tMemory operations complete. Moving onto next iteration.\r\n");
                break;
            }
        }
    }
}

static directptr_t BuddyAllocate(buddy_t* Buddy, size_t Size) {
    int InitialOrder = MAX((64 - CLZ(Size - 1)), MIN_ORDER);

    size_t WantedSize = 1ull << InitialOrder;

    if (InitialOrder >= Buddy->MaxOrder) {
        SerialPrintf("Tried to allocate too much physical memory for buddy 0x%p\r\n", Buddy);
        SerialPrintf(
                "Buddy 0x%p has max order %d, but 0x%x bytes was requested.\r\nInitial Order: %d, Wanted Size: 0x%x\r\n",
                Buddy, Buddy->MaxOrder, Size, InitialOrder, WantedSize);
        return NULL;
    }

    TicketAttemptLock(&Buddy->Lock);

    //SerialPrintf("Searching for a valid order to allocate into. Condition: {\r\n\tOrder: %d,\r\n\tSize: 0x%x\r\n}\r\n\n", InitialOrder, WantedSize);

    for (int Order = InitialOrder; Order < Buddy->MaxOrder; Order++) {
        //SerialPrintf("\tCurrent Order: %d, Buddy entry: %x\r\n", Order, Buddy->List[Order - MIN_ORDER]);
        if (Buddy->List[Order - MIN_ORDER] != 0) {
            //SerialPrintf("\tFound a valid Order!\r\n");
            directptr_t Address = Buddy->List[Order - MIN_ORDER];
            Buddy->List[Order - MIN_ORDER] = PEEK(directptr_t, Address);
            TicketUnlock(&Buddy->Lock);

            size_t FoundSize = 1ull << Order;

            //SerialPrintf("\tAdding area - Address 0x%p, Size 0x%x\r\n\n", Address, FoundSize);

            AddRangeToBuddy(Buddy, (void*) ((size_t) Address + WantedSize), FoundSize - WantedSize);

            //SerialPrintf("\tArea added!\r\n");
            return Address;
        }
    }

    //SerialPrintf("BuddyAllocate: Unable to find a valid order to allocate!\r\nInitial Order: %d, WantedSize: 0x%x\r\n\r\n", InitialOrder, WantedSize);

    TicketUnlock(&Buddy->Lock);
    return NULL;
}

void InitMemoryManager() {

    SerialPrintf("[  Mem] Counting memory..\r\n");
    size_t MemMapEntryCount = 0;

    MMapEnt* MemMap = &bootldr.mmap;

    while ((size_t) MemMap < ((size_t) &bootldr) + bootldr.size) {
        if (MMapEnt_IsFree(MemMap)) {
            FreeMemorySize += MMapEnt_Size(MemMap);
        }
        FullMemorySize += MMapEnt_Size(MemMap);
        MemMapEntryCount++;
        MemMap++;
    }

    SerialPrintf("[  Mem] Counted %d entries in the memory map..\r\n", MemMapEntryCount);

    MemoryPages = FreeMemorySize / PAGE_SIZE;

    SerialPrintf("[  Mem] %u MB of usable memory detected, %u MB total.\r\n", (FreeMemorySize / 1024) / 1024, (FullMemorySize / 1024) / 1024);
}


void ListMemoryMap() {

    SerialPrintf("[  Mem] BIOS-Provided memory map:\r\n");


    for (MMapEnt* MapEntry = &bootldr.mmap; (size_t) MapEntry < (size_t) &bootldr + bootldr.size; MapEntry++) {
        char EntryType[8] = {0};
        switch (MMapEnt_Type(MapEntry)) {
            case MMAP_FREE:
                memcpy(EntryType, "FREE", 5);
                break;
            case MMAP_USED:
                memcpy(EntryType, "RESERVED", 8);
                break;
            case MMAP_ACPI:
                memcpy(EntryType, "ACPI", 4);
                break;
            case MMAP_MMIO:
                memcpy(EntryType, "MMIO", 4);
                break;
        }

        size_t entry_from = MMapEnt_Ptr(MapEntry);
        size_t entry_to = MMapEnt_Ptr(MapEntry) + MMapEnt_Size(MapEntry);


        if (entry_from != 0 && entry_to != 0)
            SerialPrintf("[  Mem]   0x%p-0x%p %s\r\n", entry_from, entry_to, EntryType);

        if (MMapEnt_Type(MapEntry) == MMAP_FREE) {
            // We need to page align the inputs to the buddy lists.
            size_t page_from = AlignUpwards(entry_from, 0x1000);
            size_t page_to = AlignDownwards(entry_to, 0x1000);

            if (page_from != 0 && page_to != 0) {
                SerialPrintf("[  Mem]      Adding the range 0x%p-0x%p to the physical memory manager!\r\n", page_from,
                             page_to);
                AddRangeToPhysMem((void*) ((char*) (page_from)), page_to - page_from);
            }

        }
    }

}

void AddRangeToPhysMem(directptr_t Base, size_t Size) {
    if ((size_t) Base + Size < (size_t) (LOWER_REGION)) {
        SerialPrintf("[  Mem]      New range in lower memory: 0x%p, size 0x%x\r\n", Base, Size);
        AddRangeToBuddy(&LowBuddy, Base, Size);
    } else {
        if ((size_t) Base < (size_t) LOWER_REGION) {
            size_t difference = (size_t) LOWER_REGION - (size_t) Base;
            SerialPrintf(
                    "[  Mem]             Base is 0x%p bytes away from the threshold, allocating 0x%p-0x%p to lower memory..\r\n",
                    difference, (size_t) Base, (size_t) Base + difference);
            AddRangeToBuddy(&LowBuddy, Base, difference);
            Base = (void*) LOWER_REGION;
            Size = Size - difference;
        }

        if (HighBuddy.Base == NULL) {
            HighBuddy.Base = Base;
        }

        SerialPrintf("[  Mem]      New range in higher memory: 0x%p, size 0x%x\r\n", (size_t) Base, Size);
        AddRangeToBuddy(&HighBuddy, Base, Size);
    }

    if (MemoryLength < AlignUpwards((size_t) Base + Size, PAGE_SIZE) / PAGE_SIZE) {
        MemoryLength = AlignUpwards((size_t) Base + Size, PAGE_SIZE) / PAGE_SIZE;
    }
}

directptr_t PhysAllocateLowMem(size_t Size) {
    directptr_t Pointer = BuddyAllocate(&LowBuddy, Size);
    ASSERT(Pointer != NULL, "PhysAllocateLowMem: Allocation failed!");

    return Pointer;
}

directptr_t PhysAllocateMem(size_t Size) {
    directptr_t Pointer = NULL;

    if (HighBuddy.Base == 0) {
        //SerialPrintf("Attempting allocation into high memory.\n");
        Pointer = BuddyAllocate(&HighBuddy, Size);
    }

    if (Pointer == NULL) {
        //SerialPrintf("Attempting allocation into low memory.\n");
        Pointer = BuddyAllocate(&LowBuddy, Size);
    }

    ASSERT(Pointer != NULL, "PhysAllocateMem: Unable to allocate memory!");

    return Pointer;
}

directptr_t PhysAllocateZeroMem(size_t Size) {
    directptr_t Pointer = PhysAllocateMem(Size);
    memset(Pointer, 0, Size);
    return Pointer;
}

directptr_t PhysAllocateLowZeroMem(size_t Size) {
    directptr_t Pointer = PhysAllocateLowMem(Size);
    memset(Pointer, 0, Size);
    return Pointer;

}

void PhysFreeMem(directptr_t Pointer, size_t Size) {
    //ASSERT(Pointer >= (directptr_t) DIRECT_REGION, "PhysFreeMem: Attempting to free memory not in the direct mapping region.");

    buddy_t* Buddy;

    if (Pointer < (void*) (LOWER_REGION /* + DIRECT_REGION */))
        Buddy = &LowBuddy;
    else
        Buddy = &HighBuddy;

    int Order = MAX(64 - CLZ(Size - 1), MIN_ORDER);
    AddToBuddyList(Buddy, Pointer, Order, false);
}

static _Atomic(uint16_t)* PageRefCount = NULL;

void PhysAllocatorInit() {
    PageRefCount = (_Atomic(uint16_t)*) PhysAllocateZeroMem(sizeof(uint16_t) * MemoryPages);
}

directptr_t PhysAllocatePage() {
    directptr_t Page = PhysAllocateMem(PAGE_SIZE);
    PhysRefPage(Page);
    return Page;
}

void PhysRefPage(directptr_t Page) {
    PageRefCount[(size_t) Page >> PAGE_SHIFT]++;
}

void PhysFreePage(directptr_t Page) {
    if (--PageRefCount[(size_t) Page >> PAGE_SHIFT] == 0) {
        PhysFreeMem(Page, PAGE_SIZE);
    }
}

void* memcpy(void* dest, void const* src, size_t len) {
    unsigned char* dst = (unsigned char*) dest;
    const unsigned char* source = (const unsigned char*) src;

    for (size_t i = 0; i < len; i++) {
        dst[i] = source[i];
    }

    return dest;
}

void* memset(void* dst, int src, size_t len) {
    unsigned char* buf = (unsigned char*) dst;

    for (size_t i = 0; i < len; i++) {
        buf[i] = (unsigned char) src;
    }

    return dst;
}

#ifdef __cplusplus
}
#endif