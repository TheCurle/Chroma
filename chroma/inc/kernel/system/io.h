#pragma once

#include <kernel/system/descriptors.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#define PAUSE   __asm__ __volatile__("pause")

uint8_t kbdSBuffer[8];
uint8_t InputBuffer[128];

typedef struct {
    uint8_t     ACK;
    uint8_t     SelfTest;
    uint8_t     Echo;
    uint8_t     EchoCount;
    uint8_t     Error;



} KBD_FLAGS;

KBD_FLAGS KbdFlags;


DESC_TBL    ReadGDT(void);
void        WriteGDT(DESC_TBL GDTData);

DESC_TBL    ReadIDT(void);
void        WriteIDT(DESC_TBL IDTData);

uint16_t    ReadLDT(void);
void        WriteLDT(uint16_t LDTData);

uint16_t    ReadTSR(void);
void        WriteTSR(uint16_t TSRData);


uint32_t    ReadPort(uint16_t Port, int Length);
uint32_t    WritePort(uint16_t Port, uint32_t Data, int Length);
      
size_t      ReadMMIO(size_t Address, int Length);
void        WriteMMIO(size_t Address, size_t Data, int Length);

size_t      ReadModelSpecificRegister(size_t MSR);
size_t      WriteModelSpecificRegister(size_t MSR, size_t Data);

uint32_t    ReadVexMXCSR(void);
uint32_t    WriteVexMXCSR(uint32_t Data);

uint32_t    ReadMXCSR(void);
uint32_t    WriteMXCSR(uint32_t Data);

size_t      ReadControlRegister(int CRX);
size_t      WriteControlRegister(int CRX, size_t Data);

//size_t      ReadExtendedControlRegister(size_t XCRX);
size_t      WriteExtendedControlRegister(size_t XCRX, size_t Data);

void        InvalidatePage(size_t Page);

// XCS = Extended Code Segment
size_t      ReadXCS(void);

void        UpdateKeyboard(uint8_t scancode);
void        WaitFor8042();
void        Send8042(size_t);

void        WriteSerialChar(const char);
void        WriteSerialString(const char*, size_t);

int         SerialPrintf(const char* restrict format, ...);
int         Printf(const char* restrict Format, ...);

void*       memcpy(void* dest, void const* src, size_t len);
void*       memset(void* dst, int src, size_t len);