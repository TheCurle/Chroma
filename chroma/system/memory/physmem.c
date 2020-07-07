#include <kernel/chroma.h>
#include <kernel/system/heap.h>

uint8_t* Memory = ((uint8_t*)(&end));
uint8_t* MemoryStart;
size_t MemoryBuckets;


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



    for(MMapEnt* MapEntry = &bootldr.mmap; (size_t)MapEntry < (size_t)&environment; MapEntry++) {
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

        SerialPrintf("[ mem 0x%p-0x%p] %s\r\n", MMapEnt_Ptr(MapEntry), MMapEnt_Ptr(MapEntry) + MMapEnt_Size(MapEntry), EntryType);

    }

}

size_t AllocateFrame() {
    size_t FreePage = SeekFrame();
    SET_BIT(FreePage);
    return FreePage;
}

void FreeFrame(size_t Frame) {
    UNSET_BIT(Frame);
}

size_t SeekFrame() {
    for(size_t i = 0; i < MemoryPages; i++) {
        if(!READ_BIT(i))
            return i;
    }

    SerialPrintf("Memory manager: Critical!\r\n");
    return (size_t) -1;
}

void MemoryTest() {
    SerialPrintf("Initializing basic memory test..\r\n");
    bool Passed = true;
    size_t FirstPage = SeekFrame();
    /*(void* FirstPageAlloc = (void*)*/ AllocateFrame();
    size_t SecondPage = SeekFrame();
    /*void* SecondPageAlloc = (void*)*/ AllocateFrame();

    if(!(FirstPage == 0 && SecondPage == 1)) {
        Passed = false;
        SerialPrintf("First iteration: Failed, First page %x, Second page %x.\r\n", FirstPage, SecondPage);
    }
    
    FreeFrame(SecondPage);
    SecondPage = SeekFrame();

    if(SecondPage != 1)
        Passed = false;

    FreeFrame(FirstPage);
    FirstPage = SeekFrame();

    if(FirstPage != 0)
        Passed = false;

    if(Passed)
        SerialPrintf("Memory test passed.\r\n");
    else {
        SerialPrintf("Memory test failed.\r\n");
        SerialPrintf("First page %x, Second page %x.\r\n", FirstPage, SecondPage);
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


