#include <stddef.h>
#include <stdint.h>
#include <kernel/system/interrupts.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#define PAGE_SIZE 4096
#define PAGES_PER_BUCKET 8

#define OFFSET_BIT(i) Memory[i / PAGES_PER_BUCKET]
#define SET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) | (1 << (i % PAGES_PER_BUCKET))
#define UNSET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) & (~(1 << (i % PAGES_PER_BUCKET)))
#define READ_BIT(i) ((OFFSET_BIT(i) >> (i % PAGES_PER_BUCKET)) & 0x1)
#define GET_BUCKET32(i) (*((uint32_t*) (&Memory[i / 32])))

#define IS_ALIGNED(addr) (((size_t) addr | 0xFFFFFFFFFFFFF000) == 0)
#define PAGE_ALIGN(addr) ((((size_t) addr) & 0xFFFFFFFFFFFFF000) + 0x1000)

#define SET_PGBIT(cr0) (cr0 = cr0 | 1 << 31)
#define UNSET_PGBIT(cr0) (cr0 = cr0 ^ 1 << 31)
#define UNSET_PSEBIT(cr4) (cr4 = cr4 & 0xFFFFFFEF)

#define TOGGLE_PGEBIT(cr4) (cr4 = cr4 ^ (1 << 7))
#define SET_PAEBIT(cr4) (cr4 = cr4 | 1 << 5)

#define ERR_PRESENT  0x1
#define ERR_RW       0x2
#define ERR_USER     0x4
#define ERR_RESERVED 0x8
#define ERR_INST     0x10


extern size_t end;

void ListMemoryMap();

void InitMemoryManager();

size_t AllocateFrame();

void FreeFrame(size_t FrameNumber);

size_t SeekFrame();

void MemoryTest();

void InitPaging();

void PageFaultHandler(INTERRUPT_FRAME frame);