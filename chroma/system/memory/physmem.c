#include <kernel/chroma.h>
#include <kernel/system/heap.h>
#include <lainlib/lainlib.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains functions for physical memory management.
 *
 * This is also called blocking, or block memory allocation.
 * It mostly deals with the memory map handed to us by the bootloader.
 * 
 * It is useful in virtual memory management, because it allows us to map one block of physical memory to one page of virtual memory.
 * 
 * Most of the processing here is done with a bitwise mapping of blocks to allocations, normally called a memory bitmap.
 * See heap.h for the implementation.
 * 
 * This file also contains memory manipulation functions, like memset and memcpy.
 * //TODO: replace these functions with SSE2 equivalent.
 * 
 */ 


#define MIN_ORDER 3 
#define PEEK(type, address) (*((volatile type*)(address)))

uint8_t* Memory = ((uint8_t*)(&end));
uint8_t* MemoryStart;
size_t MemoryBuckets;

static buddy_t LowBuddy = {
    .MaxOrder = 32,
    .Base = (directptr_t) DIRECT_REGION,
    .List = (directptr_t[32 - MIN_ORDER]) {0},
    .Lock = {0},
};

static buddy_t HighBuddy = {
    .MaxOrder = 64,
    .Base = 0,
    .List = (directptr_t[64 - MIN_ORDER]) {0},
    .Lock = {0},
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

    /*
        SerialPrintf("About to poke memory..\r\n");
        PEEK(directptr_t, Address) = 0;
        SerialPrintf("Did it work?\r\n");
    */

    size_t Size = 1ull << Order;

    TicketAttemptLock(&Buddy->Lock);

    //SerialPrintf("Ticketlock engaged\r\n");

    if(!NewEntry && ListHead != 0) {
        directptr_t ListPrevious = 0;

        while(true) {
            if(CheckBuddies(Buddy, ListHead, Address, Size)) {
                if(ListPrevious != 0) {
                    PEEK(directptr_t, ListPrevious) = PEEK(directptr_t, ListHead);
                } else 
                    Buddy->List[Order - MIN_ORDER] = PEEK(directptr_t, ListHead);

                AddToBuddyList(Buddy, MIN(ListHead, Address), Order + 1, false);
                break;
            }

            if(PEEK(directptr_t, ListHead) == 0) {
                PEEK(directptr_t, ListHead) = Address;
                break;
            }

            ListPrevious = ListHead;
            ListHead = PEEK(directptr_t, ListHead);
        }
    } else {
        //SerialPrintf("\tAbout to poke memory 0x%p - current value is 0x%x\r\n", Address, *((size_t*)(Address)));
        *((size_t*)(Address)) = (size_t) ListHead;
        Buddy->List[Order - MIN_ORDER] = Address;
    }

    TicketUnlock(&Buddy->Lock);

    //SerialPrintf("Ticketlock Released.\r\n");
}

static void AddRangeToBuddy(buddy_t* Buddy, directptr_t Base, size_t Size) {
    //SerialPrintf("Starting a new range addition.\r\n\t");
    while(Size > (1ull << MIN_ORDER)) {
        //SerialPrintf("New iteration. Current Size: 0x%x\r\n\t", Size);
        for(int Order = Buddy->MaxOrder - 1; Order >= MIN_ORDER; Order--) {
            //SerialPrintf("New Loop. Current Order: %d\r\n\t", Order);
            if(Size >= (1ull << Order)) {
                //SerialPrintf("\tNew loop check passed.\r\n\t");
                AddToBuddyList(Buddy, Base, Order, true);
                //SerialPrintf("\tEntry added to current buddy. Moving onto memory operations..\r\n\t");
                Base = (void*)((((char*)Base) + (1ull << Order)));
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

    if(InitialOrder >= Buddy->MaxOrder) {
        SerialPrintf("Tried to allocate too much physical memory for buddy 0x%p\r\n", Buddy);
        SerialPrintf("Buddy 0x%p has max order %d, but 0x%x bytes was requested.\r\nInitial Order: %d, Wanted Size: 0x%x\r\n", Buddy, Buddy->MaxOrder, Size, InitialOrder, WantedSize);
        return NULL;
    }

    TicketAttemptLock(&Buddy->Lock);

    SerialPrintf("Searching for a valid order to allocate into. Condition: {\r\n\tOrder: %d,\r\n\tSize: 0x%x\r\n}\r\n\n", InitialOrder, WantedSize);
    
    for(int Order = InitialOrder; Order < Buddy->MaxOrder; Order++) {
        SerialPrintf("\tCurrent Order: %d, Buddy entry: %x\r\n", Order, Buddy->List[Order - MIN_ORDER]);
        if(Buddy->List[Order - MIN_ORDER] != 0) {
            SerialPrintf("\t\tFound a valid Order!\r\n");
            directptr_t Address = Buddy->List[Order - MIN_ORDER];
            Buddy->List[Order - MIN_ORDER] = PEEK(directptr_t, Address);
            TicketUnlock(&Buddy->Lock);

            size_t FoundSize = 1ull << Order;
            
            SerialPrintf("\t\tAdding area - Address 0x%p, Size 0x%x\r\n\n", Address, FoundSize);

            AddRangeToBuddy(Buddy, (void*)((size_t)Address + WantedSize), FoundSize - WantedSize);

            SerialPrintf("\t\tArea added!\r\n\n");
            return Address;
        }
    }

    SerialPrintf("BuddyAllocate: Unable to find a valid order to allocate!\r\nInitial Order: %d, WantedSize: 0x%x\r\n\r\n", InitialOrder, WantedSize);

    TicketUnlock(&Buddy->Lock);
    return NULL;
}

void InitMemoryManager() {

    size_t BootstructSize = bootldr.size;
    size_t BootstructLoc = (size_t) &bootldr;

    size_t BootstructEnd = BootstructLoc + BootstructSize;
    MemorySize = 0;
    size_t MemMapEntryCount = 0;

    MMapEnt* MemMap = &bootldr.mmap;

    while((size_t) MemMap < BootstructEnd) {
        if(MMapEnt_IsFree(MemMap)) {
            MemorySize += MMapEnt_Size(MemMap);
        }
        MemMapEntryCount++;
        MemMap++;
    }


    MemoryPages = MemorySize / PAGE_SIZE;
    MemoryBuckets = MemoryPages / PAGES_PER_BUCKET;

    if(MemoryBuckets * PAGES_PER_BUCKET < MemoryPages)
        MemoryBuckets++; // Always round up
    

    memset(Memory, 0, MemoryBuckets);

    MemoryStart = (uint8_t*) PAGE_ALIGN((size_t)(Memory + MemoryBuckets));


    SerialPrintf("Initializing Memory.\r\n");

    SerialPrintf("%u MB of memory detected.\r\n", (MemorySize / 1024) / 1024);

    for(size_t i = 0; i < MemoryBuckets; i++) {
        if(Memory[i] != 0)
            SerialPrintf("Memory at 0x%p is not empty!", Memory + i);
    }

}


void ListMemoryMap() {

    SerialPrintf("BIOS-Provided memory map:\r\n");



    for(MMapEnt* MapEntry = &bootldr.mmap; (size_t)MapEntry < (size_t)&bootldr + bootldr.size; MapEntry++) {
        char EntryType[8] = {0};
        switch(MMapEnt_Type(MapEntry)) {
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


        if(entry_from != 0 && entry_to != 0)
            SerialPrintf("[ mem 0x%p-0x%p] %s\r\n", entry_from, entry_to, EntryType);

        if(MMapEnt_Type(MapEntry) == MMAP_FREE) {
            SerialPrintf("\tAdding this entry to the physical memory manager!\r\n");
            AddRangeToPhysMem((void*)((char*)(MMapEnt_Ptr(MapEntry) /* + DIRECT_REGION*/ )), MMapEnt_Size(MapEntry));

        }
    }

}

void AddRangeToPhysMem(directptr_t Base, size_t Size) {
    if(Base < (void*)(LOWER_REGION + DIRECT_REGION)) {
        SerialPrintf("New range in lower memory: 0x%p, size 0x%x\r\n", Base, Size);
        AddRangeToBuddy(&LowBuddy, Base, Size);
    } else {
        if(HighBuddy.Base == NULL) {
            HighBuddy.Base = Base;
        }

        AddRangeToBuddy(&HighBuddy, Base, Size);
    }

    if(MemoryLength < AlignUpwards((size_t)Base + Size, PAGE_SIZE) / PAGE_SIZE) {
        MemoryLength = AlignUpwards((size_t)Base + Size, PAGE_SIZE) / PAGE_SIZE;
    }
}

directptr_t PhysAllocateLowMem(size_t Size) {
    directptr_t Pointer = BuddyAllocate(&LowBuddy, Size);
    ASSERT(Pointer != NULL, "PhysAllocateLowMem: Allocation failed!");

    return Pointer;
}

directptr_t PhysAllocateMem(size_t Size) {
    directptr_t Pointer = NULL;

    if(HighBuddy.Base == 0) 
        Pointer = BuddyAllocate(&HighBuddy, Size);

    if(Pointer == NULL)
        Pointer = BuddyAllocate(&LowBuddy, Size);
    
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
    ASSERT(Pointer >= (directptr_t) DIRECT_REGION, "PhysFreeMem: Attempting to free memory not in the direct mapping region.");

    buddy_t* Buddy;

    if(Pointer < (void*)(LOWER_REGION + DIRECT_REGION)) 
        Buddy = &LowBuddy;
    else
        Buddy = &HighBuddy;
    
    int Order = MAX(64 - CLZ(Size - 1), MIN_ORDER);
    AddToBuddyList(Buddy, Pointer, Order, false);
}

static _Atomic(uint16_t)* PageRefCount = NULL;

void PhysAllocatorInit() {
    PageRefCount = PhysAllocateZeroMem(sizeof(uint16_t) * MemoryPages);
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
    if(--PageRefCount[(size_t)Page >> PAGE_SHIFT] == 0) {
        PhysFreeMem(Page, PAGE_SIZE);
    }
}

void* memcpy(void* dest, void const* src, size_t len) {
    unsigned char* dst = (unsigned char*) dest;
    const unsigned char* source = (const unsigned char*) src;

    for(size_t i = 0; i < len; i++) {
        dst[i] = source[i];
    }

    return dest;
}

void* memset(void* dst, int src, size_t len) {
    unsigned char* buf = (unsigned char*) dst;

    for(size_t i = 0; i < len; i++) {
        buf[i] =  (unsigned char) src;
    }

    return dst;
}


