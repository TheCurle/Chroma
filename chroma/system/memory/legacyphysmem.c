
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
