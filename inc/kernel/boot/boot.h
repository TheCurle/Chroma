#pragma once
/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
;*  Memory map
;*      0h -   600h reserved for the system
 *   V----------- CAN REMOVE -----------V
;*    600h -   800h stage1 (MBR/VBR, boot.bin)
;*    800h -  6C00h stage2 (this)
 *   ∧----------- CAN REMOVE -----------∧
;*   6C00h -  7C00h stack (7000h - 700Fh SMP trampoline code)
;*   8000h -  9000h bootboot structure
;*   9000h -  A000h environment
 *   V----------- CAN REMOVE -----------V
;*   A000h -  B000h disk buffer / PML4
;*   B000h -  C000h PDPE, higher half core 4K slots
;*   C000h -  D000h PDE 4K
;*   D000h -  E000h PTE 4K
;*   E000h -  F000h PDPE, 4G physical RAM identity mapped 2M
;*   F000h - 10000h PDE 2M
;*  10000h - 11000h PDE 2M
;*  11000h - 12000h PDE 2M
;*  12000h - 13000h PDE 2M
;*  13000h - 14000h PTE 4K
 *   ∧----------- CAN REMOVE -----------∧
;*  14000h - 9F000h core stacks (1k per core)
;*
;*  At first big enough free hole, initrd. Usually at 1Mbyte.
*/

/* ELF headers hate me.
 * Let's do this the hard way. */

#define ELF_IDENT_MAGIC_OFF       0x0
#define ELF_IDENT_CLASS_OFF       0x4
#define ELF_IDENT_DATA_OFF        0x5
#define ELF_IDENT_VERSION_OFF     0x6
#define ELF_IDENT_OSABI_OFF       0x7
#define ELF_IDENT_ABI_VERSION_OFF 0x8
#define ELF_IDENT_PAD_OFF         0x9
#define ELFTYPE_OFF               0x10
#define ELFMACHINE_OFF            0x12
#define ELFVERSION_OFF            0x14
#define ELFENTRY_OFF              0x18
#define ELFPHOFF_OFF              0x20
#define ELFSHOFF_OFF              0x28 
#define ELFFLAGS_OFF              0x30
#define ELFEHSIZE_OFF             0x34
#define ELFPHENTSIZE_OFF          0x36
#define ELFPHNUM_OFF              0x38
#define ELFSHENTSIZE_OFF          0x3A
#define ELFSHNUM_OFF              0x3C
#define ELFSHSTRNDX_OFF           0x40

#define BOOT_MAGIC "BOOT"

/* minimum protocol level:
 *  hardcoded kernel name, static kernel memory addresses 
 * this is the default */
#define PROTOCOL_MINIMAL 0
/* static protocol level:
 *  kernel name parsed from environment, static kernel memory addresses */
#define PROTOCOL_STATIC  1
/* dynamic protocol level:
 *  kernel name parsed, kernel memory addresses from ELF or PE symbols */
#define PROTOCOL_DYNAMIC 2
/* big-endian flag */
#define PROTOCOL_BIGENDIAN 0x80

/* loader types, just informational */
#define LOADER_BIOS (0<<2)
/* unless we get to a weird stage, these should never be used. */
#define LOADER_UEFI (1<<2)
#define LOADER_RPI  (2<<2)

/* framebuffer pixel format, only 32 bits supported */
#define FB_ARGB   0
#define FB_RGBA   1
#define FB_ABGR   2
#define FB_BGRA   3

/* mmap entry, type is stored in least significant tetrad (half byte) of size
 * this means size described in 16 byte units (not a problem, most modern
 * firmware report memory in pages, 4096 byte units anyway). */
typedef struct {
  uint64_t   ptr;
  uint64_t   size;
} __attribute__((packed)) MMapEnt;

#define MMapEnt_Ptr(a)  (a->ptr)
#define MMapEnt_Size(a) (a->size & 0xFFFFFFFFFFFFFFF0)
#define MMapEnt_Type(a) (a->size & 0xF)
#define MMapEnt_IsFree(a) ((a->size&0xF)==1)

#define MMAP_USED     0   /* don't use. Reserved or unknown regions */
#define MMAP_FREE     1   /* usable memory */
#define MMAP_ACPI     2   /* acpi memory, volatile and non-volatile as well */
#define MMAP_MMIO     3   /* memory mapped IO region */

#define INITRD_MAXSIZE 16 /* Mb */

typedef struct {
  /* first 64 bytes is platform independent */
  uint8_t    magic[4];    /* 'BOOT' magic */
  uint32_t   size;        /* length of bootboot structure, minimum 128 */
  uint8_t    protocol;    /* 1, static addresses, see PROTOCOL_* and LOADER_* above */
  uint8_t    fb_type;     /* framebuffer type, see FB_* above */
  uint16_t   numcores;    /* number of processor cores */
  uint16_t   bspid;       /* Bootstrap processor ID (Local APIC Id on x86_64) */
  int16_t    timezone;    /* in minutes -1440..1440 */
  uint8_t    datetime[8]; /* in BCD yyyymmddhhiiss UTC (independent to timezone) */
  uint64_t   initrd_ptr;  /* ramdisk image position and size */
  uint64_t   initrd_size;
  uint8_t    *fb_ptr;     /* framebuffer pointer and dimensions */
  uint32_t   fb_size;
  uint32_t   fb_width;
  uint32_t   fb_height;
  uint32_t   fb_scanline;

  /* the rest (64 bytes) is platform specific */
  union {
    struct {
      uint64_t acpi_ptr;
      uint64_t smbi_ptr;
      uint64_t efi_ptr;
      uint64_t mp_ptr;
      uint64_t unused0;
      uint64_t unused1;
      uint64_t unused2;
      uint64_t unused3;
    } x86_64;
    struct {
      uint64_t acpi_ptr;
      uint64_t mmio_ptr;
      uint64_t efi_ptr;
      uint64_t unused0;
      uint64_t unused1;
      uint64_t unused2;
      uint64_t unused3;
      uint64_t unused4;
    } aarch64;
  } arch;

  /* from 128th byte, MMapEnt[], more records may follow */
  MMapEnt    mmap;
  /* use like this:
   * MMapEnt *mmap_ent = &bootboot.mmap; mmap_ent++;
   * until you reach bootboot->size */
} __attribute__((packed)) bootinfo;



#ifdef  __cplusplus
}
#endif
