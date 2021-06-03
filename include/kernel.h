/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* ==================== Kernel Header ==================== */
/* This file contains most of the shared code for the kernel and
 * the classes it interacts with.
 * 
 * Most of this file is constants and struct definitions. */

#pragma once

#define SYNC_MAJOR_VERSION 1
#define SYNC_MINOR_VERSION 0


/*  ==================== Standard Headers ==================== */
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <x86intrin.h>

#include <efibind.h>
#include <efitypes.h>
#include <efierr.h>

/*  ==================== Custom Headers ==================== */
#include <bitmapfont.h>


/*  ==================== EFI structs and constants ==================== */

/*  ========= Graphics ========== */
typedef struct {
    uint32_t       RedMask;
    uint32_t       GreenMask;
    uint32_t       BlueMask;
    uint32_t       ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueRedGreenReserved8BitPerColor,
    PixelBitMask,
    PixelBitOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    uint32_t                    Version;
    uint32_t                    HorizontalResolution;
    uint32_t                    VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT   PixelFormat;
    EFI_PIXEL_BITMASK           PixelInformation;
    uint32_t                    PixelsPerScanline;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    uint32_t                                MaxMode;
    uint32_t                                Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*   Info;
    size_t                                  SizeOfInfo;
    EFI_PHYSICAL_ADDRESS                    FrameBufferBase;
    size_t                                  FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

/* ========== Time ========== */

typedef struct {
    uint32_t        Resolution;
    uint32_t        Accuracy;
    BOOLEAN         SetsToZero;
} EFI_TIME_CAPABILITIES;

typedef EFI_STATUS (EFIAPI* EFI_GET_TIME) (
    OUT EFI_TIME*                   Time,
    OUT EFI_TIME_CAPABILITIES*      Capabilities OPTIONAL
);

typedef EFI_STATUS (EFIAPI* EFI_SET_TIME) (
    IN EFI_TIME*                    Time
);

typedef EFI_STATUS (EFIAPI* EFI_GET_WAKEUP_TIME) (
    OUT BOOLEAN*        Enabled,
    OUT BOOLEAN*        Pending,
    OUT EFI_TIME*       Time
);

typedef EFI_STATUS (EFIAPI* EFI_SET_WAKEUP_TIME) (
    IN BOOLEAN          Enable,
    IN EFI_TIME*        Time OPTIONAL
);

/* ========== Variables ========== */

#define EFI_GLOBAL_VARIABLE     \
    { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} }


#define EFI_VARIABLE_NON_VOLATILE                          0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS                    0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS                        0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD                 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS            0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE                          0x00000040

typedef EFI_STATUS (EFIAPI* EFI_GET_VARIABLE) (
    IN CHAR16*          VariableName,
    IN EFI_GUID*        VendorGuid,
    OUT uint32_t*       Attributes OPTIONAL,
    IN OUT size_t*      DataSize,
    OUT void*           Data
);

typedef EFI_STATUS (EFIAPI* EFI_GET_NEXT_VARIABLE_NAME) (
    IN OUT size_t*      VariableNameSize,
    IN OUT CHAR16*      VariableName,
    IN OUT EFI_GUID*    VendorGuid
);

typedef EFI_STATUS (EFIAPI* EFI_SET_VARIABLE) (
    IN CHAR16*          VariableName,
    IN EFI_GUID*        VendorGuid,
    IN uint32_t         Attributes,
    IN size_t           DataSize,
    IN void*            Data
);

/* ========== Memory ========== */

typedef EFI_STATUS (EFIAPI* EFI_SET_VIRTUAL_ADDRESS_MAP) (
    IN size_t                   MemoryMapSize,
    IN size_t                   DescriptorSize,
    IN uint32_t                 DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR*   VirtualMap
);


#define EFI_OPTIONAL_PTR            0x00000001
#define EFI_INTERNAL_FNC            0x00000002     
#define EFI_INTERNAL_PTR            0x00000004      

typedef EFI_STATUS (EFIAPI* EFI_CONVERT_POINTER) (
    IN size_t           DebugDeposition,
    IN OUT void*       *Address
);

/* ========== Reset ========== */

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown
} EFI_RESET_TYPE;

typedef EFI_STATUS (EFIAPI* EFI_RESET_SYSTEM) (
    IN EFI_RESET_TYPE       ResetType,
    IN EFI_STATUS           ResetStatus,
    IN size_t               DataSize,
    IN CHAR16*              ResetData OPTIONAL
);


/* ========== Other ========== */

typedef EFI_STATUS(EFIAPI* EFI_GET_NEXT_HIGH_MONO_COUNT) (
    OUT uint32_t*       HighCount
);


/* ===== Capsule ===== */

typedef struct {
    EFI_GUID    CapsuleGuid;
    uint32_t    HeaderSize;
    uint32_t    Flags;
    uint32_t    CapsuleImageSize;
} EFI_CAPSULE_HEADER;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET      0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE     0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET            0x00040000

typedef EFI_STATUS (EFIAPI* EFI_UPDATE_CAPSULE) (
    IN EFI_CAPSULE_HEADER*     *CapsuleHeaderArray,
    IN size_t                   CapsuleCount,
    IN EFI_PHYSICAL_ADDRESS     ScatterGatherList OPTIONAL
);

typedef EFI_STATUS (EFIAPI* EFI_QUERY_CAPSULE_CAPABILITIES) (
    IN EFI_CAPSULE_HEADER*     *CapsuleHeaderArray,
    IN size_t                   CapsuleCount,
    OUT size_t*                 MaximumCapsuleSize,
    OUT EFI_RESET_TYPE*         ResetType
);

typedef EFI_STATUS (EFIAPI* EFI_QUERY_VARIABLE_INFO) (
    IN uint32_t     Attributes,
    OUT size_t*     MaximumVariableStorage,
    OUT size_t*     RemainingVariableStorageSize,
    OUT size_t*     MaximumVariableSize
);


/*  ==================== EFI Runtime Services ==================== */

typedef struct {
    EFI_TABLE_HEADER                    Header;

    /* Time */
    EFI_GET_TIME                        GetTime;
    EFI_SET_TIME                        SetTime;
    EFI_GET_WAKEUP_TIME                 GetWakeupTime;
    EFI_SET_WAKEUP_TIME                 SetWakeupTime;

    /* Virtual Memory */
    EFI_SET_VIRTUAL_ADDRESS_MAP         SetVirtualAddressMap;
    EFI_CONVERT_POINTER                 ConvertPointer;

    /* Variable Services */
    EFI_GET_VARIABLE                    GetVariable;
    EFI_GET_NEXT_VARIABLE_NAME          GetNextVariableName;
    EFI_SET_VARIABLE                    SetVariable;
    
    /* Other */
    EFI_GET_NEXT_HIGH_MONO_COUNT        GetNextHighMonoCount;
    EFI_RESET_SYSTEM                    ResetSystem;

    /* Capsule Services */
    EFI_UPDATE_CAPSULE                  UpdateCapsule;
    EFI_QUERY_CAPSULE_CAPABILITIES      QueryCapsuleCapabilities;
    EFI_QUERY_VARIABLE_INFO             QueryVariableInfo;
    
} EFI_RUNTIME_SERVICES;


/*  ==================== EFI File Info ==================== */

typedef struct {
    size_t      Size;
    size_t      FileSize;
    size_t      PhysicalSize;
    EFI_TIME    CreateTime;
    EFI_TIME    LastAccessTime;
    EFI_TIME    ModificationTime;
    size_t      Attribute;
    CHAR16      FileName[1];
} EFI_FILE_INFO;

/*  ==================== EFI System Configuration Tables ==================== */


#define MPS_TABLE_GUID    \
    { 0xeb9d2d2f, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define ACPI_10_TABLE_GUID    \
    { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define ACPI_20_TABLE_GUID  \
    { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }

#define SMBIOS_TABLE_GUID    \
    { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define SMBIOS3_TABLE_GUID    \
    { 0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94} }

#define SAL_SYSTEM_TABLE_GUID    \
    { 0xeb9d2d32, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

static const EFI_GUID MpsTableGuid       = MPS_TABLE_GUID;
static const EFI_GUID Acpi10TableGuid    = ACPI_10_TABLE_GUID;
static const EFI_GUID Acpi20TableGuid    = ACPI_20_TABLE_GUID;
static const EFI_GUID SmBiosTableGuid    = SMBIOS_TABLE_GUID;
static const EFI_GUID SmBios3TableGuid   = SMBIOS3_TABLE_GUID;
static const EFI_GUID SalSystemTableGuid = SAL_SYSTEM_TABLE_GUID;

typedef struct _EFI_CONFIGURATION_TABLE {
    EFI_GUID        VendorGuid;
    void*           VendorTable;
} EFI_CONFIGURATION_TABLE;

/*  ============================================================= */
/*  =                    Syncboot Structures                    = */
/*  ============================================================= */

typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* GPUs;
    size_t                             FBCount;
} GPU_INFO;

typedef struct {
    uint32_t                    UEFI_Version;
    uint32_t                    SYNCBOOT_Major;
    uint32_t                    SYNCBOOT_Minor;

    uint32_t                    MemoryMapDescriptorVersion;
    size_t                      MemoryMapDescriptorSize;
    EFI_MEMORY_DESCRIPTOR*      MemoryMap;
    size_t                      MemoryMap_Size;
    EFI_PHYSICAL_ADDRESS        KernelBase;
    size_t                      KernelPages;

    CHAR16*                     ESPRootPath;
    size_t                      ESPRootPathSize;
    CHAR16*                     KernelPath;
    size_t                      KernelPathSize;
    CHAR16*                     KernelOptions;
    size_t                      KernelOptionsSize;

    EFI_RUNTIME_SERVICES*       RTServices;
    GPU_INFO*                   GPU_Info;
    EFI_FILE_INFO*              KernelFileMetadata;
    EFI_CONFIGURATION_TABLE*    ConfigurationTables;
    size_t                      NumConfigurationTables;
} FILELOADER_PARAMS;


/*  ============================================================= */
/*  =                      Sync Structures                      = */
/*  ============================================================= */


typedef struct {
    uint32_t                MemoryMapDescriptorVersion;
    size_t                  MemoryMapDescriptorSize;
    size_t                  MemoryMapSize;
    EFI_MEMORY_DESCRIPTOR*  MemoryMap;
    uint32_t                Pad;
} MEMORY_INFO;

typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE   defaultGPU;
    uint32_t                            charHeight;
    uint32_t                            charWidth;   // Bit width
    uint32_t                            charFGColor; // ForeGround Color
    uint32_t                            charHLColor; // HighLight Color
    uint32_t                            charBGColor; // BackGround Color
    uint32_t                            screenMinX;  // Far left side of the screen
    uint32_t                            screenMinY;  // Top of the screen
    uint32_t                            charScale;
    uint32_t                            cursorPos;
    uint32_t                            scrollMode;  // Move screen up, or wraparound
} PRINT_INFO;

/*  ==================== GDT ==================== */



typedef struct __attribute__((packed)) {
	uint16_t	LowLimit;
	uint16_t	LowBase;
	uint8_t		MiddleBase;
	uint8_t		Access;
	uint8_t		Granularity;
	uint8_t		HighBase;
} GDT_ENTRY;

typedef struct __attribute__((packed)) {
	uint16_t 	Limit;
	size_t		BaseAddress;
} DESCRIPTOR_TABLE_POINTER;

GDT_ENTRY gdt[3]; //3-entry gdt
DESCRIPTOR_TABLE_POINTER gtdp;


/*  ==================== IDT ==================== */


typedef struct __attribute__((packed)) {
	uint16_t	LowBase;
	uint16_t	Segment;
	uint8_t		IST;
	uint8_t		SegmentType;
	uint16_t	MiddleBase;
	uint32_t	HighBase;
	uint32_t	Reserved;
} IDT_GATE;

typedef struct __attribute__((packed)) {
	uint16_t	LowBase;
	uint16_t	Selector;
	uint8_t		IST;
	uint8_t		Flags;
	uint16_t	HighBase;
} IDT_ENTRY;

IDT_ENTRY idt[256]; 
DESCRIPTOR_TABLE_POINTER idtp; // IDT Pointer

/*  ==================== TSS ==================== */

/* Intel Architecture Manual Vol. 3A, Fig. 7-4 (Format of TSS and LDT Descriptors in 64-bit Mode)
 *
 * Segment Limit is a 20-bit value - the lower 16 bits are stored at SegmentLimitLow and the higher 4 bits at SegmentLimitHigh;
 * Base Address is a 64-bit value - The lower 24 bits are stored at Low and Middle1, the upper 46 bits at Middle2 and High.
 * SegmentType stores the following:
 * 
 *  =========================
 *  | 0  -  3 | 4 | 5 6 | 7 |
 *  | SEGType | S | DPL | P |
 *  =========================
 * 
 * SegmentLimitHigh stores the following:
 * 
 *  ==========================================
 *  | 0     -     3 |   4   | 5 |  6  |   7  |
 *  | SegmentLimit  | Avail | L | D/B | Gran |
 *  ==========================================
 * 
*/
typedef struct __attribute__((packed)) {
	uint16_t	SegmentLimitLow;
	uint16_t 	BaseLow;
  	uint8_t  	BaseMiddle1;
  	uint8_t  	SegmentType;
  	uint8_t  	SegmentLimitHigh;
  	uint8_t  	BaseMiddle2;
  	uint32_t 	BaseHigh;
 	uint32_t	Reserved;
} TSS_ENTRY;

typedef struct __attribute__((packed)) {
	uint32_t	SegmentSelector;
	uint16_t	SegmentType;
	uint16_t	SegmentLow;
	uint32_t	SegmentHigh;
	uint32_t	Reserved;
} CALL_GATE_ENTRY;

/* Intel Architecture Manual Vol. 3A, Fig. 7-11 (64-Bit TSS Format)
 */
typedef struct __attribute__((packed)) {
	uint32_t	Reserved0;
	size_t		RSP0;
	size_t		RSP1;
	size_t		RSP2;

	size_t		Reserved12;

	size_t		IST1;
	size_t		IST2;
	size_t		IST3;
	size_t		IST4;
	size_t		IST5;
	size_t		IST6;
	size_t		IST7;

	size_t		Reserved34;

	uint16_t	Reserved5;
	uint16_t	IOMap;

} TSS_64;


/*  ==================== ACPI ==================== */

/* 
 * ACPI Specification 6.2A, section 5.2.5 (Root System Description Pointer (RSDP)) 
 */

typedef struct __attribute__((packed)) {
	char		Signature[8]; // "RSD PTR "
	uint8_t		Checksum;
	char		OEMID[6];
	uint8_t		Revision;
	uint32_t	RSDT; // 32 bit
} RSDP_10;

typedef struct __attribute__((packed)) {
	RSDP_10		RSDP_10_Section;
	uint32_t	Length;
	size_t		XSDT; // = 64 bit RSDT
	uint8_t		ChecksumExtended;
	uint8_t		Reserved[3];
} RSDP_20;

typedef struct __attribute__((packed)) {
	char		Signature[4];
	uint32_t	Length;
	uint8_t		Revision;
	uint8_t		Checksum;
	char		OEMID[6];
	char		OEMTableID[8];
	uint32_t	OEMRevision;
	uint32_t	CreatorID;
	uint32_t	CreatorRevision;
} SDTHeader; // System Descriptor Table

typedef struct __attribute__((packed)) {
	SDTHeader	SDTHeader;
	size_t		Entry[1];
} XSDT;


/*  ==================== Global Variables ==================== */

MEMORY_INFO	Memory_Info = {0};
PRINT_INFO 	Print_Info = {0};

uint8_t		Image[96] = {0};
char		BrandStr[48] = {0};
char		ManufacturerStr[13] = {0};

/*  ==================== Interrupts ==================== */

typedef struct __attribute__((packed)) {
    size_t  rip;
    size_t  cs;
    size_t  rflags;
    size_t  rsp;
    size_t  ss;
} INTERRUPT_FRAME;

typedef struct __attribute__((packed)) {
    size_t ErrorCode;
    size_t rip;
    size_t cs;
    size_t rflags;
    size_t rsp;
    size_t ss;
} EXCEPTION_FRAME;

typedef struct __attribute__((aligned(64), packed)) {
    // Legacy FXSAVE header
    UINT16 fcw;
    UINT16 fsw;
    UINT8 ftw;
    UINT8 Reserved1;
    UINT16 fop;
    UINT64 fip;     
    UINT64 fdp; 
    UINT32 mxcsr;
    UINT32 mxcsr_mask;

    // Legacy x87/MMX registers
    UINT64 st_mm_0[2];
    UINT64 st_mm_1[2];
    UINT64 st_mm_2[2];
    UINT64 st_mm_3[2];
    UINT64 st_mm_4[2];
    UINT64 st_mm_5[2];
    UINT64 st_mm_6[2];
    UINT64 st_mm_7[2];

    // SSE registers
    UINT64 xmm0[2];
    UINT64 xmm1[2];
    UINT64 xmm2[2];
    UINT64 xmm3[2];
    UINT64 xmm4[2];
    UINT64 xmm5[2];
    UINT64 xmm6[2];
    UINT64 xmm7[2];
    UINT64 xmm8[2];
    UINT64 xmm9[2];
    UINT64 xmm10[2];
    UINT64 xmm11[2];
    UINT64 xmm12[2];
    UINT64 xmm13[2];
    UINT64 xmm14[2];
    UINT64 xmm15[2];
    UINT8 Reserved2[48];
    UINT8 pad[48];
    UINT64 xstate_bv; 
    UINT64 xcomp_bv;
    UINT64 Reserved3[6];
    UINT8 extended_region[1];
} XSAVE_AREA;


static void* IRQ_Handlers[16];

static const char* ExceptionStrings[] = {
	"Division by Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint", 
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};


/*  ============================================================= */
/*  =                       Sync Functions                      = */
/*  ============================================================= */

int         kernel_main(FILELOADER_PARAMS* FLOP);

/* Required functions */
size_t      strlen(const char* string);

/* System Initialization */
void        PrepareSystem(FILELOADER_PARAMS* FLOP);

/* System Utilities */
size_t      ClockTick(void);
void        panic(void);
void        PrepareAVX(void);
void        AVXStub(void);
void        PrepareMaskableInterrupts(void);
void        PreparePowerManagement(void);

/* Hypervisor / Scheduler */
uint8_t     CheckForHypervisor(void);
uint8_t     ReadPerformance(size_t* PerformanceDest);
size_t      ReadCPUFrequency(size_t* PerformanceDest, uint8_t AvgOrDirect);

/* Read/Write Port (eg. Serial) */
uint32_t    ReadPort(uint16_t Port, int Length);
uint32_t    WritePort(uint16_t Port, uint32_t Data, int Length);

/* Serial functions */
void        serialWrite(const char chr);
void        serialPrint(const char* data);
void        serialPrintf(const char* format, ...);
void        serialInit();
void        inttoa(int input, char* output);
void        itoh(int input, char* output);
void        zeroString(char* string);

/*  ==================== Registers ==================== */
size_t      ReadModelSpecificRegister(size_t MSR);
size_t      WriteModelSpecificRegister(size_t MSR, size_t Data);

uint32_t    ReadVexMXCSR(void);
uint32_t    WriteVexMXCSR(uint32_t Data);

uint32_t    ReadMXCSR(void);
uint32_t    WriteMXCSR(uint32_t Data);

size_t      ReadControlRegister(int CRX);
size_t      WriteControlRegister(int CRX, size_t Data);

size_t      ReadExtendedControlRegister(size_t XCRX);
size_t      WriteExtendedControlRegister(size_t XCRX, size_t Data);

// XCS = Extended Code Segment
size_t      ReadXCS(void);

/*  ==================== Descriptor Tables ==================== */
DESCRIPTOR_TABLE_POINTER    FetchGDT(void);
void                        SetGDT(DESCRIPTOR_TABLE_POINTER GDTData);

DESCRIPTOR_TABLE_POINTER    FetchIDT(void);
void                        SetIDT(DESCRIPTOR_TABLE_POINTER IDTData);

uint16_t                    FetchLDT(void);
void                        SetLDT(uint16_t LDTData);

uint16_t                    FetchTSR(void);
void                        SetTSR(uint16_t TSRData);

void    InstallGDT(void);
void    InstallIDT(void);
void    InstallPaging(void);

/*  ==================== Branding ==================== */
char*   FetchBrandStr(uint32_t* Str);
char*   FetchManufacturer(char* ID);

void    ScanCPUFeatures(size_t RAX, size_t RCX);

/*  ==================== Interrupts ==================== */

static uint64_t time;

void IRQ_Common(INTERRUPT_FRAME* Frame, size_t Interupt);
void ISR_Common(INTERRUPT_FRAME* Frame, size_t Interrupt);
void ISR_Error_Common(EXCEPTION_FRAME* Frame, size_t Exception);

void RemapIRQControllers();

void ISR0Handler(INTERRUPT_FRAME* Frame); // Divide-By-Zero
void ISR1Handler(INTERRUPT_FRAME* Frame); // Debug
void ISR2Handler(INTERRUPT_FRAME* Frame); // Non-Maskable Interrupt
void ISR3Handler(INTERRUPT_FRAME* Frame); // Breakpoint
void ISR4Handler(INTERRUPT_FRAME* Frame); // Overflow
void ISR5Handler(INTERRUPT_FRAME* Frame); // Out-of-Bounds
void ISR6Handler(INTERRUPT_FRAME* Frame); // Invalid Opcode
void ISR7Handler(INTERRUPT_FRAME* Frame); // No Coprocessor
void ISR8Handler(EXCEPTION_FRAME* Frame); // Double Fault
void ISR9Handler(INTERRUPT_FRAME* Frame); // Coprocessor Overrun
void ISR10Handler(EXCEPTION_FRAME* Frame); // Invalid TSS
void ISR11Handler(EXCEPTION_FRAME* Frame); // Segment Not Present
void ISR12Handler(EXCEPTION_FRAME* Frame); // Stack Segment Fault
void ISR13Handler(EXCEPTION_FRAME* Frame); // General Protection Fault
void ISR14Handler(EXCEPTION_FRAME* Frame); // Page Fault
void ISR15Handler(INTERRUPT_FRAME* Frame); // Unknown Interrupt
void ISR16Handler(INTERRUPT_FRAME* Frame); // Math Error / Coprocessor Fault
void ISR17Handler(EXCEPTION_FRAME* Frame); // Alignment Error
void ISR18Handler(INTERRUPT_FRAME* Frame); // Machine Check
void ISR19Handler(INTERRUPT_FRAME* Frame); // SSE Math Error
void ISR20Handler(INTERRUPT_FRAME* Frame); // Virtualization Exception
void ISR30Handler(EXCEPTION_FRAME* Frame); // Security Fault
void ReservedISRHandler(INTERRUPT_FRAME* Frame);

/*  ==================== Interrupt Support ==================== */

void DumpRegisters_ISR(INTERRUPT_FRAME* Frame);
void DumpRegisters_Error(EXCEPTION_FRAME* Frame);
void DumpRegisters_AVX(XSAVE_AREA* Area);
void DumpControlRegisters(void);
void DumpFileloaderParams(FILELOADER_PARAMS* FLOP);
void DumpSegmentRegisters(void);

void irq_install_handler(int irq, void (*handler)(INTERRUPT_FRAME* r));
void irq_uninstall_handler(int);
void irq_install();
void timer_install();

/*  ==================== Memory ==================== */

// Standard GCC reqs
void* memmove (void* dest, const void* src, size_t len);
void memset(void* src, int chr, size_t n);
void memcpy(void* dest, void* src, size_t n);
int memcmp (const void* str1, const void* str2, size_t count);


uint8_t VerifyZero(size_t Length, size_t Start);

size_t  FetchMemoryLimit(void);
size_t  FetchVisibleMemory(void);
size_t  FetchAvailableMemory(void);
size_t  FetchAvailablePMemory(void);
size_t  FetchInstalledMemory(void);

void PrintMemoryMap();

EFI_MEMORY_DESCRIPTOR* SetIdentityMap(EFI_RUNTIME_SERVICES* RT);
void InstallMemoryMap(void);

void ReclaimEfiBootServicesMemory(void);
void ReclaimEfiLoaderCodeMemory(void);

void MergeFragmentedMemory(void);

EFI_PHYSICAL_ADDRESS PurgeAllMemory(void);
EFI_PHYSICAL_ADDRESS AllocatePagetable(size_t PagetableSize);


/*     AVX     */
void* memmoveAVX(void* Dest, void* Source, size_t Length);
void* memcpyAVX(void* Dest, void* Source, size_t Length);
void* memsetAVX(void* Dest, const uint8_t Source, size_t Length);
void* memsetAVX_By4Bytes(void* Dest, const uint32_t Source, size_t Length);
uint8_t memcmpAVX(const void* String1, const void* String2, size_t Length);
#define CACHESIZE 3*1024*1024


/*     Physical Addresses     */
__attribute__((malloc)) void* kalloc(size_t Length);
__attribute__((malloc)) void* kalloc_16(size_t Length);
__attribute__((malloc)) void* kalloc_32(size_t Length);
__attribute__((malloc)) void* kalloc_64(size_t Length);
__attribute__((malloc)) void* kalloc_pages(size_t Pages);

EFI_PHYSICAL_ADDRESS FindFreeAddress(size_t Pages, EFI_PHYSICAL_ADDRESS Start);
EFI_PHYSICAL_ADDRESS FindFreeAddress_ByPage(size_t Pages, EFI_PHYSICAL_ADDRESS Start);

/* The _ByXBytes functions here try to align the allocated space to the specified size. */
EFI_PHYSICAL_ADDRESS AllocateFreeAddress_ByPage(size_t Pages, EFI_PHYSICAL_ADDRESS Start);
EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By16Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS Start);
EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By32Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS Start);
EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By64Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS Start);

/*     Virtual Addresses     */

__attribute__((malloc)) void* valloc(size_t Length);
__attribute__((malloc)) void* valloc_16(size_t Length);
__attribute__((malloc)) void* valloc_32(size_t Length);
__attribute__((malloc)) void* valloc_64(size_t Length);
__attribute__((malloc)) void* valloc_pages(size_t Pages);

EFI_VIRTUAL_ADDRESS FindVirtualAddress(size_t Pages, EFI_VIRTUAL_ADDRESS Start);
EFI_VIRTUAL_ADDRESS FindVirtualAddress_ByPage(size_t Pages, EFI_VIRTUAL_ADDRESS Start);

EFI_VIRTUAL_ADDRESS AllocateVirtualAddress_ByPage(size_t Pages, EFI_VIRTUAL_ADDRESS Start);
EFI_VIRTUAL_ADDRESS AllocateVirtualAddress_By16Bytes(size_t Pages, EFI_VIRTUAL_ADDRESS Start);
EFI_VIRTUAL_ADDRESS AllocateVirtualAddress_By32Bytes(size_t Pages, EFI_VIRTUAL_ADDRESS Start);
EFI_VIRTUAL_ADDRESS AllocateVirtualAddress_By64Bytes(size_t Pages, EFI_VIRTUAL_ADDRESS Start);

/*  ==================== Graphics ==================== */

void ClearScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU);
void FillScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU, uint32_t Color);

void ResetFillScreen(void);
void ResetScreen(void);

void DrawPixel(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                uint32_t x,
                uint32_t y,
                uint32_t Color);

void BitmapSetup(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                const uint8_t* Bitmap,
                uint32_t Height,
                uint32_t BitWidth,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale);

void BitmapRender(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                const uint8_t* BitMap,
                uint32_t Height,
                uint32_t BitWidth,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale,
                uint32_t Character);

void BitmapBitswap(const uint8_t* BitMap,
                uint32_t Height,
                uint32_t BitWidth, 
                uint8_t* Out);

void BitmapReverse(const uint8_t* BitMap,
                uint32_t Height,
                uint32_t BitWidth,
                uint8_t* Out);

void BitmapMirror(const uint8_t* BitMap,
                uint32_t Height,
                uint32_t BitWidth,
                uint8_t* Out);


/*  ========== Text ========== */

void SetupPrinting(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU);

void WriteChar(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                int Char,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor);

void WriteCharPos(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                int Char,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y);

void WriteScaledChar(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                int Char,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale);

void WriteScaledString(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                const char* string,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale);

void WriteScaledFormatString(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale,
                const char* String,
                ...);

void RenderText(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE GPU,
                int Char,
                uint32_t Height,
                uint32_t Width,
                uint32_t FontColor,
                uint32_t HighlightColor,
                uint32_t x,
                uint32_t y,
                uint32_t Scale,
                uint32_t Index);


/*  ==================== Printf ==================== */

int snprintf(char* String, size_t Length, const char* Format, ...);
int vsnprintf(char* String, size_t Length, const char* Format, va_list AP);
int vsnrprintf(char* String, size_t Length, int Radix, const char* Format, va_list AP);
int sprintf(char* Buffer, const char* Format, ...);
int vsprintf(char* Buffer, const char* Format, va_list AP);

int printf(const char* Format, ...);
int vprintf(const char* Format, va_list AP);
int kvprintf(const char* Format, void (*func)(int, void*), void* Args, int Radix, va_list AP);

void PrintUTF16AsUTF8(CHAR16* String, size_t Length);
char* UTF16ToUTF8(CHAR16* String, size_t Length);

void gdb_end();