#include <kernel/chroma.h>

uint8_t* Memory = ((uint8_t*)(&end));
uint8_t MemoryStart;
uint32_t MemoryPages;
uint32_t MemoryLength;

void InitMemoryManager() {

    size_t BootstructSize = bootldr.size;
    size_t BootstructLoc = (size_t) &bootldr;

    size_t BootstructEnd = BootstructLoc + BootstructSize;
    size_t MemorySize = 0, MemMapEntryCount = 0;

    MMapEnt* MemMap = &bootldr.mmap;

    while((size_t) MemMap < BootstructEnd) {
        if(MMapEnt_IsFree(MemMap)) {
            MemorySize += MMapEnt_Size(MemMap);
        }
        MemMapEntryCount++;
        MemMap++;
    }


    MemoryPages = MemorySize / PAGE_SIZE;
    MemoryLength = MemoryPages / PAGES_PER_BUCKET;

    if(MemoryLength * PAGES_PER_BUCKET < MemoryPages)
        MemoryLength++; // Always round up
    

    memset(Memory, 0, MemoryLength);

    MemoryStart = (uint8_t*)PAGE_ALIGN(((uint32_t)(Memory + MemoryLength)));


    SerialPrintf("Initializing Memory.\r\n");

    SerialPrintf("%u MB of memory detected.\r\n", (MemorySize / 1024) / 1024);

    for(size_t i = 0; i < MemoryLength; i++) {
        if(Memory[i] != 0)
            SerialPrintf("Memory at 0x%p is not empty!", Memory + i);
    }

}

size_t AllocatePage() {
    size_t FreePage = FirstFreePage();
    SET_BIT(FreePage);
    return FreePage;
}

void FreePage(size_t Page) {
    UNSET_BIT(Page);
}

size_t FirstFreePage() {
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
    size_t FirstPage = FirstFreePage();
    void* FirstPageAlloc = AllocatePage();
    size_t SecondPage = FirstFreePage();
    void* SecondPageAlloc = AllocatePage();

    if(!(FirstPage == 0 && SecondPage == 1)) {
        Passed = false;
        SerialPrintf("First iteration: Failed, First page %x, Second page %x.\r\n", FirstPage, SecondPage);
    }
    
    FreePage(SecondPage);
    SecondPage = FirstFreePage();

    if(SecondPage != 1)
        Passed = false;

    FreePage(FirstPage);
    FirstPage = FirstFreePage();

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


