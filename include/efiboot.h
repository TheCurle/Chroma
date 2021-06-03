#pragma once

#include <efi/efi.h>
#include <efi/efilib.h>
#include <stddef.h>

#define MAJOR_VER 2
#define MINOR_VER 5

#ifdef DEBUG
	#define METADATA_SHOW
	#define WATCHDOG_TIMER_DISABLE
	#define MAIN_DEBUG
	#define GFX_DEBUG_MAIN
	#define GFX_DEBUG_NAMING
	#define LOADER_DEBUG_MAIN
	#define LOADER_DEBUG_PE
	#define LOADER_DEBUG_DOS
	#define LOADER_DEBUG_ELF
	#define LOADER_DEBUG_FINAL
	#define MEMORY_SHOW
	#define MEMORY_CHECK
#endif

/* ==================== Text File Magic Numbers ==================== */
/* Required to make sure that the configuration file is encoded properly. 
 * The magic number is set automatically by all text editors, including Notepad. */

#define UTF8_BOM_LE 0xBFBBEF // Little Endian
#define UTF8_BOM_BE 0xEFBBBF // Big Endian

#define UTF16_BOM_LE 0xFEFF
#define UTF16_BOM_BE 0xFFFE


/* ==================== Fileloader Structs ==================== */

/* These are the structs passed to the kernel file when it's called by the bootloader.
 * More information about this can be found in main.c */

typedef struct {
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE*	GPUs;		// Information about each graphics output unit (one or more per GPU)
	size_t								FBCount;    // The amount of framebuffers available.
} GFX_INFO;

typedef struct {
	uint32_t					UEFI_Version;					// Version of the firmware running
	uint32_t					Bootloader_MajorVersion;		// Major and
	uint32_t					Bootloader_MinorVersion;		// minor version of the bootloader itself.

	uint32_t					Memory_Descriptor_Version;
	size_t						Memory_Descriptor_Size;			// Size of each memory descriptor
	EFI_MEMORY_DESCRIPTOR*		Memory_Map;						// System memory map, as an array of EFI_MEMORY_DESCRIPTORs
	size_t						Memory_Map_Size;				// Total size of the memory map

	EFI_PHYSICAL_ADDRESS		Kernel_Base;					// The physical (absolute) address of the start of the kernel
	size_t						Kernel_Pages;					// How many pages (of 4KiB) the kernel takes up

	CHAR16*						ESP_Path;						// The drive root of the EFI System Partition
	size_t						ESP_Path_Length;				// Size in bytes of the ESP_Path string
	CHAR16*						Kernel_Path;					// Kernel's file path relative to the ESP_Path
	size_t						Kernel_Path_Length;				// Size in bytes of the Kernel_Path string
	CHAR16*						Kernel_Options;					// Options given to the kernel, taken from the 2nd line of the 
	size_t						Kernel_Options_Length;			// Size in bytes of the Kernel_Options string

	EFI_RUNTIME_SERVICES*		RTServices;						// The UEFI Runtime Services
	GFX_INFO*					GPU_INFO;						// Information about all available graphics devices
	EFI_FILE_INFO*				FileMeta;						// Kernel file metadata
	EFI_CONFIGURATION_TABLE*	ConfigTables;					// UEFI system configuration tables
	size_t						ConfigTables_Length;
} FILELOADER_PARAMS;

static const CHAR16 pixelFormats[5][17] = {
	L"RGBReserved 8bpp",
	L"BGRReserved 8bpp",
	L"PixelBitMask    ",
	L"PixelBltOnly    ",
	L"PixelFormatMax  "
};

uint8_t Language[6] = { 'e','n','-','U','S','\0' };
uint8_t Language2[3] = { 'e','n','\0' };
uint8_t Language3[4] = { 'e','n','g','\0' };

CHAR16 DefaultDriverName[10] = L"No Driver";
CHAR16 DefaultControllerName[14] = L"No Controller";
CHAR16 DefaultChildName[9] = L"No Child";


/* ==================== Sanity Checks ==================== */

#define PROBLEM_DRIVERS 4 // There are 4 known drivers that claim to control anything you ask it. This causes problems.

const CHAR16 AmiPS2Driver[16] = L"AMI PS/2 Driver"; // This driver controls a "Generic PS/2 Keyboard"
const CHAR16 AsixUSBEthDriver[34] = L"ASIX AX88772B Ethernet Driver 1.0"; // This driver controls "ASIX AX88772B USB Fast Ethernet Controller"
const CHAR16 SocketLayerDriver[20] = L"Socket Layer Driver"; // This driver controls a "Socket Layer"
const CHAR16 Asix10100EthernetDriver[24] = L"AX88772 Ethernet Driver"; // This driver controls "AX88772 10/100 Ethernet"
const CHAR16* const Problematic_Drivers[PROBLEM_DRIVERS] = { AmiPS2Driver, AsixUSBEthDriver, SocketLayerDriver, Asix10100EthernetDriver };

