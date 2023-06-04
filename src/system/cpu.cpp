#include <kernel/chroma.h>
#include <kernel/system/interrupts.h>
#include "driver/io/apic.h"

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
 * 
 */

extern size_t __interrupt_vector[128];

#define IDT_INTERRUPT 0x8e
#define IDT_USER 0x60

void InvalidatePage(size_t Page) {
    __asm__ __volatile__("invlpg (%%eax)" : : "a" (Page));
}

__attribute__((aligned(64))) static volatile size_t InitGDT[5] = {
        0,
        0x00af9a000000ffff,
        0x00cf92000000ffff,
        0x0080890000000067,
        0
};

__attribute__((aligned(64))) static DESC_TBL CoreGDT = {
        .Limit = sizeof(InitGDT),
        .Base = (size_t) InitGDT
};

__attribute__((aligned(64))) static volatile TSS64 TSSEntries;

__attribute__((aligned(64))) static volatile IDT_GATE IDTEntries[256];

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

void SetupInitialGDT() {
    auto TSSBase = (uint64_t) (&TSSEntries);

    auto TSSLower = (uint16_t) TSSBase;
    auto TSSMid1 = (uint8_t) (TSSBase >> 16);
    auto TSSMid2 = (uint8_t) (TSSBase >> 24);
    auto TSSHigher = (uint32_t) (TSSBase >> 32);


    ((TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]))->BaseLow = TSSLower;
    ((TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]))->BaseMid1 = TSSMid1;
    ((TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]))->BaseMid2 = TSSMid2;
    ((TSS_ENTRY*) (&((GDT_ENTRY*) InitGDT)[3]))->BaseHigh = TSSHigher;

    WriteGDT(CoreGDT);
    WriteTSR(3 << 3);
    RefreshCS();
}

static void SetIDT(size_t vector, size_t handler, size_t IST, size_t type) {
    auto ISRBaseLow = (uint16_t) handler;
    auto ISRBaseMid = (uint16_t) (handler >> 16);
    auto ISRBaseHigh = (uint32_t) (handler >> 32);

    IDTEntries[vector].BaseLow = ISRBaseLow;
    IDTEntries[vector].Segment = 0x08;
    IDTEntries[vector].ISTAndZero = IST;
    IDTEntries[vector].SegmentType = type;
    IDTEntries[vector].BaseMid = ISRBaseMid;
    IDTEntries[vector].BaseHigh = ISRBaseHigh;
    IDTEntries[vector].Reserved = 0;
}


void SetupIDT() {
    DESC_TBL IDTData;
    IDTData.Limit = (sizeof(IDT_GATE) * 256) - 1;
    IDTData.Base = (size_t) &IDTEntries;

    for (int i = 0; i < 48; i++) {
        if (i == 14 || i == 32)
            SetIDT(i, (size_t)__interrupt_vector[i], 1, IDT_INTERRUPT);
        else
            SetIDT(i, (size_t)__interrupt_vector[i], 0, IDT_INTERRUPT);
    }

    // TODO: Choose two interrupts for process switching.

    for (auto& IRQHandler : IRQHandlers) {
        IRQHandler = {{}, 0 };
    }

    WriteIDT(IDTData);
}