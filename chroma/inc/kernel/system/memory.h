#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGES_PER_BUCKET 8

#define OFFSET_BIT(i) Memory[i / PAGES_PER_BUCKET]
#define SET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) | (1 << (i % PAGES_PER_BUCKET))
#define UNSET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) & (~(1 << (i % PAGES_PER_BUCKET)))
#define READ_BIT(i) ((OFFSET_BIT(i) >> (i % PAGES_PER_BUCKET)) & 0x1)
#define GET_BUCKET32(i) (*((uint32_t*) (&Memory[i / 32])))

#define PAGE_ALIGN(addr) (((addr) & 0xFFFFF000) + 0x1000)

extern size_t end;

void InitMemoryManager();

size_t AllocatePage();

void FreePage(size_t PageNumber);

size_t FirstFreePage();

void MemoryTest();