#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file serves to allow us to communicate with the computer through raw I/O.
 * It provides interfaces for Ports and commonly used Registers (Control Registers, Model-Specific Registers, GDT, IDT..)
 * 
 * Additionally, there are wrapper functions for MMIO accesses.
 * 
 */

uint32_t ReadPort(uint16_t Port, int Length) {
    uint32_t Data = 0;
    uint16_t Data16 = 0;
    uint8_t  Data8 = 0;

    if(Length == 1) {
        __asm__ __volatile__ ("inb %[address], %[value]" : [value] "=a" (Data8) : [address] "d" (Port) :);
        Data = (uint32_t) Data8;
    } else if(Length == 2) {
        __asm__ __volatile__ ("inw %[address], %[value]" : [value] "=a" (Data16) : [address] "d" (Port) :);
        Data = (uint32_t) Data16;
    } else if(Length == 4)
        __asm__ __volatile__ ("inl %[address], %[value]" : [value] "=a" (Data) : [address] "d" (Port) :);
    //else
        //printf("Readport: Invalid length\r\n");

    return Data;
}

uint32_t WritePort(uint16_t Port, uint32_t Data, int Length) {
    if(Length == 1)
        __asm__ __volatile__ ("outb %[value], %[address]" : : [value] "a" ((uint8_t) Data), [address] "d" (Port) :);
    else if(Length == 2)
        __asm__ __volatile__ ("outw %[value], %[address]" : : [value] "a" ((uint16_t) Data), [address] "d" (Port) :);
    else if(Length == 4)
        __asm__ __volatile__ ("outl %[value], %[address]" : : [value] "a" (Data), [address] "d" (Port) :);
    //else
        //printf("WritePort: Invalid length.\r\n");

    return Data;
}

size_t ReadMMIO(size_t Address, int Length) {
    if (Length == 1) {
        return *((volatile uint8_t*)(Address));
    } else if (Length == 2) {
        return *((volatile uint16_t*)(Address));
    } else if (Length == 4) {
        return *((volatile uint32_t*)(Address));
    } else {
        return *((volatile size_t*)(Address));
    }
}

void WriteMMIO(size_t Address, size_t Data, int Length) {
    if(Length == 1) {
        (*((volatile uint8_t*)(Address))) = ((uint8_t) Data);
    } else if (Length == 2) {
        (*((volatile uint16_t*)(Address))) = ((uint16_t) Data);
    } else if (Length == 4) {
        (*((volatile uint32_t*)(Address))) = ((uint32_t) Data);
    } else {
        (*((volatile uint8_t*)(Address))) = (Data);
    } 
}


size_t ReadModelSpecificRegister(size_t MSR) {
    size_t RegHigh = 0, RegLow = 0;

    __asm__ __volatile__ ("rdmsr" : "=a" (RegLow), "=d" (RegHigh) : "c" (MSR) :);

    return RegHigh << 32 | RegLow;
}

size_t WriteModelSpecificRegister(size_t MSR, size_t Data) {

    const size_t DataLow = Data & 0x00000000ffffffff;
    const size_t DataHigh = (Data & 0xffffffff00000000) >> 32;

    __asm__ __volatile__ ("wrmsr" : : "a" (DataLow), "c" (MSR), "d" (DataHigh) :);

    return Data;
}

uint32_t ReadVexMXCSR() {
    uint32_t Data;

    __asm__ __volatile__ ("vstmxcsr %[dest]" : [dest] "=m" (Data) : :);
    return Data;
}

uint32_t WriteVexMXCSR(uint32_t Data) {
    
    __asm__ __volatile__ ("vldmxcsr %[src]" : : [src] "m" (Data) :);
    return Data;
}

uint32_t ReadMXCSR() {
    uint32_t Data;

    __asm__ __volatile__ ("stmxcsr %[dest]" : [dest] "=m" (Data) : :);
    return Data;
}

uint32_t WriteMXCSR(uint32_t Data) {

    __asm__ __volatile__ ("ldmxcsr %[src]" : : [src] "m" (Data) :);
    return Data;
}

size_t ReadControlRegister(int CRX) {

    size_t Data;

    switch(CRX) {
        case 0:
            __asm__ __volatile__ ("mov %%cr0, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 1:
            __asm__ __volatile__ ("mov %%cr1, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 2:
            __asm__ __volatile__ ("mov %%cr2, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 3:
            __asm__ __volatile__ ("mov %%cr3, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 4:
            __asm__ __volatile__ ("mov %%cr4, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 8:
            __asm__ __volatile__ ("mov %%cr8, %[dest]" : [dest] "=r" (Data) : :);
            break;
        case 'f':
            __asm__ __volatile__ ("pushfq\n\t" "popq %[dest]" : [dest] "=r" (Data) : :);
            break;
        default:
            SerialPrintf("invalid crx read %x\r\n",CRX);
            Data = 0xdeadbeef;
            break;
    }

    return Data;
}

size_t WriteControlRegister(int CRX, size_t Data) {
    switch(CRX) {
        case 0:
            __asm__ __volatile__ ("movq %[dest], %%cr0" : : [dest] "r" (Data) : );
            break;
        case 1:
            __asm__ __volatile__ ("movq %[dest], %%cr1" : : [dest] "r" (Data) : );
            break;
        case 2:
            __asm__ __volatile__ ("movq %[dest], %%cr2" : : [dest] "r" (Data) : );
            break;
        case 3:
            __asm__ __volatile__ ("movq %[dest], %%cr3" : : [dest] "r" (Data) : );
            break;
        case 4:
            __asm__ __volatile__ ("movq %[dest], %%cr4" : : [dest] "r" (Data) : );
            break;
        case 8:
            __asm__ __volatile__ ("movq %[dest], %%cr8" : : [dest] "r" (Data) : );
            break;
        case 'f':
            __asm__ __volatile__ ("pushq %[dest]\n\t" "popfq" : : [dest] "r" (Data) : "cc");
            break;
        default:
            break;
    }

    return Data;
}

size_t ReadExtendedControlRegister(size_t XCRX) {
    size_t RegHigh = 0, RegLow = 0;

    __asm__ __volatile__("xgetbv" : "=a" (RegLow), "=d" (RegHigh) : "c" (XCRX) :);

    return (RegHigh << 32 | RegLow);
}

//TODO: make this just have an assert as returning data for a write to catch
// errors that shoudlunt really happen doesent make alot of sense
size_t WriteExtendedControlRegister(size_t XCRX, size_t Data){
    uint32_t DataLow =   Data & 0x00000000ffffffff;
    uint32_t DataHigh = (Data & 0xffffffff00000000) >> 32;
    __asm__ __volatile__("xsetbv" : : "a" (DataLow), "c" (XCRX), "d" (DataHigh) :);
    return Data;
}

size_t ReadXCS() {
    size_t Data;

    __asm__ __volatile__("mov %%cs, %[dest]" : [dest] "=r" (Data) : :);
    return Data;
}

DESC_TBL ReadGDT() {
    DESC_TBL GDTData = {0};

    __asm__ __volatile__("sgdt %[dest]" : [dest] "=m" (GDTData) : :);

    return GDTData;
}

void WriteGDT(DESC_TBL GDTData) {

    __asm__ __volatile__("lgdt %[src]" : : [src] "m" (GDTData) :);
}

DESC_TBL ReadIDT() {
    DESC_TBL IDTData = {0}; 

    __asm__ __volatile__("sidt %[dest]" : [dest] "=m" (IDTData) : :);

    return IDTData;
}

void WriteIDT(DESC_TBL IDTData) {

    __asm__ __volatile__("lidt %[src]" : : [src] "m" (IDTData) :);
}

uint16_t ReadLDT() {
    uint16_t LDTData;

    __asm__ __volatile__("sldt %[dest]" : [dest] "=m" (LDTData) : :);

    return LDTData;
}

void WriteLDT(uint16_t LDTData) {

    __asm__ __volatile__("lldt %[src]" : : [src] "m" (LDTData) :);
}

uint16_t ReadTSR() {
    uint16_t TSRData;

    __asm__ __volatile__("str %[dest]" : [dest] "=m" (TSRData) : :);

    return TSRData;
}

void WriteTSR(uint16_t TSRData) {

    __asm__ __volatile__("ltr %[src]" : : [src] "m" (TSRData) :);
}


