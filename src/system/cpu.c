#include <kernel/chroma.h>
#include <kernel/system/interrupts.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This class provides functions for setting up and preparing the CPU for the things the kernel will do.
 * Mainly, it allows you to:
 * 
 * Set up and install the GDT and IDT
 * Refresh the Code Segment to force ourselves into our own GDT
 * Install new ISR and IRQ handlers.

 * It also has (unused) functionality for extra stacks, to be used with Non-Maskable Interrupt, Double Fault, Machine Check and Breakpoint Exceptions.
 * //TODO
 * 
 */

#define NMI_STACK 4096
#define DF_STACK 4096
#define MC_STACK 4096
#define BP_STACK 4096

void InvalidatePage(size_t Page) {
    __asm__ __volatile__("invlpg (%%eax)" : : "a" (Page) );
}

__attribute__((aligned(64))) static volatile unsigned char NMIStack[NMI_STACK] = {0};
__attribute__((aligned(64))) static volatile unsigned char DFStack[DF_STACK] = {0};
__attribute__((aligned(64))) static volatile unsigned char MCStack[MC_STACK] = {0};
__attribute__((aligned(64))) static volatile unsigned char BPStack[BP_STACK] = {0};

__attribute__((aligned(64))) static volatile size_t InitGDT[5] = {
    0,
    0x00af9a000000ffff,
    0x00cf92000000ffff,
    0x0080890000000067,
    0
};

__attribute__((aligned(64))) static volatile TSS64 TSSEntries = {0};

__attribute__((aligned(64))) static volatile IDT_GATE IDTEntries[256] = {0};

static void RefreshCS() {
    
    __asm__ __volatile__ ("mov $16, %ax \n\t" // 16 = 0x10 = index 2 of GDT
                          "mov %ax, %ds \n\t" // Next few lines prepare the processor for the sudden change into the data segment
                          "mov %ax, %es \n\t" //
                          "mov %ax, %fs \n\t" //
                          "mov %ax, %gs \n\t" //
                          "mov %ax, %ss \n\t" //
                          "movq $8, %rdx \n\t" // 8 = 0x8 = index 1 of GDT
                          "leaq 4(%rip), %rax \n\t" // Returns execution to immediately after the iret, required for changing %cs while in long mode. - this is currently literally the only way to do it.
                          "pushq %rdx \n\t"   //
                          "pushq %rax \n\t"   //
                          "lretq \n\t");      //
}


void PrepareCPU() {

    SetupInitialGDT();
    SetupIDT();

    //SetupExtensions();

    InitInterrupts();

}

/*void SetupExtensions() {

    // Enable SSE
    size_t CR0 = ReadControlRegister(0);

    CR0 &= ~(1 << 2);
    CR0 |= 1;

    WriteControlRegister(0, CR0);

    // Enable OSXSAVE and gang
    size_t CR4 = ReadControlRegister(4);
    CR4 |= (1 << 9);
    CR4 |= (1 << 10);
    CR4 |= (1 << 18);

    WriteControlRegister(4, CR4);

    // Enable AVX (and AVX-512 in future)
    
    CR0 = ReadExtendedControlRegister(0);
    SerialPrintf("XCR0 is currently %x.\n", CR0);
    CR0 |= (1 << 0);
    CR0 |= (1 << 1);
    CR0 |= (1 << 2);

    CR0 |= (1 << 5);
    CR0 |= (1 << 6);
    CR0 |= (1 << 7);

    SerialPrintf("About to write xcr0: %x\n", CR0);
    WriteExtendedControlRegister(0, CR0);
}
*/

void SetupInitialGDT() {
    DESC_TBL GDTData = {0};
    size_t TSSBase = (uint64_t) (&TSSEntries);

    uint16_t TSSLower = (uint16_t) TSSBase;
    uint8_t TSSMid1 = (uint8_t) (TSSBase >> 16);
    uint8_t TSSMid2 = (uint8_t) (TSSBase >> 24);
    uint32_t TSSHigher = (uint32_t) (TSSBase >> 32);

    GDTData.Limit = sizeof(InitGDT) - 1;
    GDTData.Base = (size_t) InitGDT;

    ( (TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]) )->BaseLow = TSSLower;
    ( (TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]) )->BaseMid1 = TSSMid1;
    ( (TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]) )->BaseMid2 = TSSMid2;
    ( (TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]) )->BaseHigh = TSSHigher;

    WriteGDT(GDTData);
    WriteTSR(3 << 3);
    RefreshCS();
}

static void SetISR(size_t ISRNum, size_t ISRAddr) {
    uint16_t ISRBaseLow = (uint16_t) ISRAddr;
    uint16_t ISRBaseMid = (uint16_t) (ISRAddr >> 16);
    uint32_t ISRBaseHigh = (uint32_t) (ISRAddr >> 32);

    IDTEntries[ISRNum].BaseLow = ISRBaseLow;
    IDTEntries[ISRNum].Segment = 0x08;
    IDTEntries[ISRNum].ISTAndZero = 0;
    IDTEntries[ISRNum].SegmentType = 0x8E;
    IDTEntries[ISRNum].BaseMid = ISRBaseMid;
    IDTEntries[ISRNum].BaseHigh = ISRBaseHigh;
    IDTEntries[ISRNum].Reserved = 0;
}

static void SetISRNMI(size_t ISRNum, size_t ISRAddr) {
    uint16_t ISRBaseLow = (uint16_t) ISRAddr;
    uint16_t ISRBaseMid = (uint16_t) (ISRAddr >> 16);
    uint32_t ISRBaseHigh = (uint32_t) (ISRAddr >> 32);

    IDTEntries[ISRNum].BaseLow = ISRBaseLow;
    IDTEntries[ISRNum].Segment = 0x08;
    IDTEntries[ISRNum].ISTAndZero = 1;
    IDTEntries[ISRNum].SegmentType = 0x8E;
    IDTEntries[ISRNum].BaseMid = ISRBaseMid;
    IDTEntries[ISRNum].BaseHigh = ISRBaseHigh;
    IDTEntries[ISRNum].Reserved = 0;
}

static void SetISRDF(size_t ISRNum, size_t ISRAddr) {
    uint16_t ISRBaseLow = (uint16_t) ISRAddr;
    uint16_t ISRBaseMid = (uint16_t) (ISRAddr >> 16);
    uint32_t ISRBaseHigh = (uint32_t) (ISRAddr >> 32);

    IDTEntries[ISRNum].BaseLow = ISRBaseLow;
    IDTEntries[ISRNum].Segment = 0x08;
    IDTEntries[ISRNum].ISTAndZero = 2;
    IDTEntries[ISRNum].SegmentType = 0x8E;
    IDTEntries[ISRNum].BaseMid = ISRBaseMid;
    IDTEntries[ISRNum].BaseHigh = ISRBaseHigh;
    IDTEntries[ISRNum].Reserved = 0;
}

static void SetISRMC(size_t ISRNum, size_t ISRAddr) {
    uint16_t ISRBaseLow = (uint16_t) ISRAddr;
    uint16_t ISRBaseMid = (uint16_t) (ISRAddr >> 16);
    uint32_t ISRBaseHigh = (uint32_t) (ISRAddr >> 32);

    IDTEntries[ISRNum].BaseLow = ISRBaseLow;
    IDTEntries[ISRNum].Segment = 0x08;
    IDTEntries[ISRNum].ISTAndZero = 3;
    IDTEntries[ISRNum].SegmentType = 0x8E;
    IDTEntries[ISRNum].BaseMid = ISRBaseMid;
    IDTEntries[ISRNum].BaseHigh = ISRBaseHigh;
    IDTEntries[ISRNum].Reserved = 0;
}

static void SetISRBP(size_t ISRNum, size_t ISRAddr) {
    uint16_t ISRBaseLow = (uint16_t) ISRAddr;
    uint16_t ISRBaseMid = (uint16_t) (ISRAddr >> 16);
    uint32_t ISRBaseHigh = (uint32_t) (ISRAddr >> 32);

    IDTEntries[ISRNum].BaseLow = ISRBaseLow;
    IDTEntries[ISRNum].Segment = 0x08;
    IDTEntries[ISRNum].ISTAndZero = 4;
    IDTEntries[ISRNum].SegmentType = 0x8E;
    IDTEntries[ISRNum].BaseMid = ISRBaseMid;
    IDTEntries[ISRNum].BaseHigh = ISRBaseHigh;
    IDTEntries[ISRNum].Reserved = 0;
}

void SetupIDT() {
    DESC_TBL IDTData = {0};
    IDTData.Limit = (sizeof(IDT_GATE) * 256) - 1;
    IDTData.Base = (size_t) &IDTEntries;

    //memset(&IDTEntries, 0, sizeof(IDT_GATE) * 256);

	RemapIRQControllers();

    *(size_t*) (&TSSEntries.IST1) = (size_t) (NMIStack + NMI_STACK);
    *(size_t*) (&TSSEntries.IST2) = (size_t) (DFStack + DF_STACK);
    *(size_t*) (&TSSEntries.IST3) = (size_t) (MCStack + MC_STACK);
    *(size_t*) (&TSSEntries.IST4) = (size_t) (BPStack + BP_STACK);

    SetISR      (0, (size_t) ISR0Handler);
    SetISRBP    (1, (size_t) ISR1Handler);
    SetISRNMI   (2, (size_t) ISR2Handler);
    SetISRBP    (3, (size_t) ISR3Handler);
    SetISR      (4, (size_t) ISR4Handler);
    SetISR      (5, (size_t) ISR5Handler);
    SetISR      (6, (size_t) ISR6Handler);
    SetISR      (7, (size_t) ISR7Handler);
    SetISRDF    (8, (size_t) ISR8Handler);
    SetISR      (9, (size_t) ISR9Handler);
    SetISR      (10, (size_t) ISR10Handler);
    SetISR      (11, (size_t) ISR11Handler);
    SetISR      (12, (size_t) ISR12Handler);
    SetISR      (13, (size_t) ISR13Handler);
    SetISR      (14, (size_t) ISR14Handler);
    SetISR      (15, (size_t) ISR15Handler);
    SetISR      (16, (size_t) ISR16Handler);
    SetISR      (17, (size_t) ISR17Handler);
    SetISRMC    (18, (size_t) ISR18Handler);
    SetISR      (19, (size_t) ISR19Handler);
    SetISR      (20, (size_t) ISR20Handler);
    SetISR      (30, (size_t) ISR30Handler);

    for(size_t i = 1; i < 11; i++) {
        if(i != 9) {
            SetISR(i + 20, (size_t) ReservedISRHandler);
        }
    }

    SetISR(32, (size_t) IRQ0Handler);
    SetISR(33, (size_t) IRQ1Handler);
    SetISR(34, (size_t) IRQ2Handler);
    SetISR(35, (size_t) IRQ3Handler);
    SetISR(36, (size_t) IRQ4Handler);
    SetISR(37, (size_t) IRQ5Handler);
    SetISR(38, (size_t) IRQ6Handler);
    SetISR(39, (size_t) IRQ7Handler);
    SetISR(40, (size_t) IRQ8Handler);
    SetISR(41, (size_t) IRQ9Handler);
    SetISR(42, (size_t) IRQ10Handler);
    SetISR(43, (size_t) IRQ11Handler);
    SetISR(44, (size_t) IRQ12Handler);
    SetISR(45, (size_t) IRQ13Handler);
    SetISR(46, (size_t) IRQ14Handler);
    SetISR(47, (size_t) IRQ15Handler);


    //TODO: ISRs 32 to 256

    WriteIDT(IDTData);

}



