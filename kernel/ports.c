
/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* This file contains code required for directly communicating with hardware.
 * This means ASM, most of the time. These are just wrapper functions that make ASM a little bit less daunting.
 */


#include <kernel.h>


uint32_t ReadPort(uint16_t Port, int Length) {
    uint32_t Data;
    if(Length == 1) { // Read a byte
        __asm__ __volatile__("inb %[address], %[value]" : : [value] "a" ((uint8_t)  Data), [address] "d" (Port) :); 
    } else if (Length == 2) { // Read a word
        __asm__ __volatile__("inw %[address], %[value]" : : [value] "a" ((uint16_t) Data), [address] "d" (Port) :); 
    } else if (Length == 4) { // Read a long (dword)
        __asm__ __volatile__("inl %[address], %[value]" : : [value] "a" (Data), [address] "d" (Port) :); 
    } else {
        printf("ReadPort: Invalid Read Length.\r\n");
    }

    return Data;
}

uint32_t WritePort(uint16_t Port, uint32_t Data, int Length) {
    if(Length == 1) { // Write a byte
         __asm__ __volatile__("outb %[value], %[address]" : : [value] "a" ((uint8_t) Data), [address] "d" (Port) :); 
    } else if (Length == 2) { // Write a word
        __asm__ __volatile__("outw %[value], %[address]" : : [value] "a" ((uint16_t) Data), [address] "d" (Port) :); 
    } else if (Length == 4) { // Write a long (dword)
        __asm__ __volatile__("outl %[value], %[address]" : : [value] "a" (Data), [address] "d" (Port) :); 
    } else {
        printf("WritePort: Invalid Write Length.\r\n");
    }

    return Data;
}

size_t ReadModelSpecificRegister(size_t MSR) {
    size_t RegHigh = 0, RegLow = 0;
    __asm__ __volatile__("rdmsr" : "=a" (RegLow), "=d" (RegHigh) : "c" (MSR) :);
    return (RegHigh << 32 | RegLow);
}

size_t WriteModelSpecificRegister(size_t MSR, size_t Data) {
    size_t DataLow = 0, DataHigh = 0;

    DataLow = ((uint32_t* )&Data)[0];
    DataHigh = ((uint32_t* )&Data)[1];
    __asm__ __volatile__("wrmsr" : : "a" (DataLow), "c" (MSR), "d" (DataHigh) : );
    return Data;
}

// VMXCSR - Vex-Encoded MXCSR. These are preferred when AVX is available.
uint32_t ReadVexMXCSR() {
    uint32_t Data;
    __asm__ __volatile__("vstmxcsr %[dest]" : [dest] "=m" (Data) : :);
    return Data;
}

uint32_t WriteVexMXCSR(uint32_t Data) {
    __asm__ __volatile__("vldmxcsr %[src]" : : [src] "m" (Data) :);
    return Data;
}

// MXCSR - SSE Control Register.
uint32_t ReadMXCSR() {
    uint32_t Data;
    __asm__ __volatile__("stmxcsr %[dest]" : [dest] "=m" (Data) : :);
    return Data;
}

uint32_t WriteMXCSR(uint32_t Data) {
    __asm__ __volatile__("ldmxcsr %[src]" : : [src] "m" (Data) :);
    return Data;
}

// Control Register : CRX + RFLAGS. Specify 'f' for RFLAGS, X for CRX.

size_t ReadControlRegister(int CRX) {
    size_t Data;
    switch(CRX) {
        case 0:
            __asm__ __volatile__("mov %%cr0, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 1:
            __asm__ __volatile__("mov %%cr1, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 2:
            __asm__ __volatile__("mov %%cr2, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 3:
            __asm__ __volatile__("mov %%cr3, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 4:
            __asm__ __volatile__("mov %%cr4, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 8:
            __asm__ __volatile__("mov %%cr8, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 'f':
            // Push flags and pop them into our buffer
            __asm__ __volatile__("pushfq\n\t" "popq %[dest]" : [dest] "=r" (Data) : :);
            break;
        default:
            break;
    }

    return Data;
}

size_t WriteControlRegister(int CRX, size_t Data) {
    switch(CRX) {
        case 0:
            __asm__ __volatile__("mov %[dest], %%cr0" : : [dest] "r" (Data) :);
            break;
        case 1:
            __asm__ __volatile__("mov %[dest], %%cr1" : : [dest] "r" (Data) :);
            break;
        case 2:
            __asm__ __volatile__("mov %[dest], %%cr2" : : [dest] "r" (Data) :);
            break;
        case 3:
            __asm__ __volatile__("mov %[dest], %%cr3" : : [dest] "r" (Data) :);
            break;
        case 4:
            __asm__ __volatile__("mov %[dest], %%cr4" : : [dest] "r" (Data) :);
            break;
        case 8:
            __asm__ __volatile__("mov %[dest], %%cr8" : : [dest] "r" (Data) :);
            break;
        case 'f':
            __asm__ __volatile__("pushq %[dest]\n\t" "popfq" : : [dest] "r" (Data) : "cc");
            break;

        default:
            break;
    }

    return Data;

}

// XCR = eXtended Control Register.
// XCR0 is used to enable AVX/SSE.

size_t ReadExtendedControlRegister(size_t XCRX) {
    size_t RegHigh = 0, RegLow = 0;

    __asm__ __volatile__("xgetbv" : "=a" (RegLow), "=d" (RegHigh) : "c" (XCRX) :);
    return (RegHigh << 32 | RegLow);
}

size_t WriteExtendedControlRegister(size_t XCRX, size_t Data) {
    __asm__ __volatile__("xsetbv" : : "a" ( ((uint32_t*)&Data)[0]), "c" (XCRX), "d" ( ((uint32_t*)&Data)[1] ) :);
    return Data;
}

// The following two functions are utility - for determining whether we're operating in Long Mode.
// TODO: Move into DescriptorTables.c

size_t ReadXCS() {
    size_t Data = 0;
    __asm__ __volatile__("mov %%cs, %[dest]" : [dest] "=r" (Data) : :);
    return Data;
}

DESCRIPTOR_TABLE_POINTER FetchGDT() {
    DESCRIPTOR_TABLE_POINTER GDTrData = {0};

    __asm__ __volatile__("sgdt %[dest]" : [dest] "=m" (GDTrData) : :);
    return GDTrData;
}

void SetGDT(DESCRIPTOR_TABLE_POINTER GDTrData) {
    __asm__ __volatile__("lgdt %[src]" : : [src] "m" (GDTrData) :);
}

DESCRIPTOR_TABLE_POINTER FetchIDT() {
    DESCRIPTOR_TABLE_POINTER IDTrData = {0};
    __asm__ __volatile__("sidt %[dest]" : [dest] "=m" (IDTrData) : :);
    return IDTrData;
}

void SetIDT(DESCRIPTOR_TABLE_POINTER IDTrData) {
    __asm__ __volatile__("lidt %[src]" : : [src] "m" (IDTrData) :);
}

// LDT = Local Descriptor Table (= GDT entry for current segment)

uint16_t FetchLDT() {
    uint16_t LDTrData = 0;
    __asm__ __volatile__("sldt %[dest]" : [dest] "=m" (LDTrData) : :);
    return LDTrData;
}

void SetLDT(uint16_t LDTrData) {
    __asm__ __volatile__("lldt %[src]" : : [src] "m" (LDTrData) :);
}

// TSR - Tast State Register

uint16_t FetchTSR() {
    uint16_t TSRData = 0;
    __asm__ __volatile__ ("str %[dest]" : [dest] "=m" (TSRData) : :);
    return TSRData;
}

void SetTSR(uint16_t TSRData) {
    __asm__ __volatile__("ltr %[src]" : : [src] "m" (TSRData) :);
}
