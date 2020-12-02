#include <stddef.h>
#include <stdint.h>
#include <kernel/system/interrupts.h>
#include <lainlib/lainlib.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/


/************************************************
*    C O N S T A N T S   A N D   M A C R O S
*************************************************/

#define PAGE_SIZE 4096
#define PAGES_PER_BUCKET 8

#define OFFSET_BIT(i) Memory[i / PAGES_PER_BUCKET]
#define SET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) | (1 << (i % PAGES_PER_BUCKET))
#define UNSET_BIT(i) OFFSET_BIT(i) = OFFSET_BIT(i) & (~(1 << (i % PAGES_PER_BUCKET)))
#define READ_BIT(i) ((OFFSET_BIT(i) >> (i % PAGES_PER_BUCKET)) & 0x1)
#define GET_BUCKET32(i) (*((uint32_t*) (&Memory[i / 32])))

#define CAST(a, b)  ((a) (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

#define REINTERPRET_CAST(target, intermediate, value) ((target*)((intermediate*)value))

#define FIXENDIAN64(x) __builtin_bswap64(x)
#define FIXENDIAN32(x) __builtin_bswap32(x)
#define FIXENDIAN16(x) __builtin_bswap16(x)


#define CONCAT(x, y) x ## y
#define CONCAT2(x, y) CONCAT(x, y)
#define ASSERT(exp, error) \
        if(!(exp)) SomethingWentWrong(error);
       // typedef char CONCAT2(static_assert, __LINE__) [(exp) ? 1 : -1]

#define CLZ(num) (num ? __builtin_clzll(num) : 64)

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

#define ELF64MAGIC   0x7F454c46
#define ELF64MAGICBE 0x464c457F


/*
 * The way we boot, using BOOTBOOT, and the static hard drive images, means 
 * we're limited to Protocol 1 - we cannot ask the bootloader to move anything
 * around for us. 
 * 
 * That means we need to account for these unmovable sections in the paging system.
 * 
 * MMIO_REGION 
 *      Represents the MMIO symbol defined in the linkerscript and chroma.h.
 * FB_REGION
 *      Represents the framebuffer used throughout the kernel.
 *      This is likely the most important thing to keep where it is. Without this, we
 *       have no video output.
 * KERNEL_REGION
 *      This is where the kernel itself is loaded into memory. Protocol 1 means 
 *       we're loaded into the -2MB area.
 *        We *CAN* mvoe the kernel about in memory. It's as simple as memcpying it around
 *         and calling a void pointer as a function to return to where we were. 
 *        We *CANNOT* move the framebuffer in this manner, as it is set directly by BIOS,
 *         and the graphics device most likely will not allow this to happen.
 *        For this reason, the kernel, framebuffer and MMIO will remain where they are.
 *         Luckily, there are more components of Chroma than the kernel itself. That's what
 *          the kernel heap and kernel stack areas are for.
 * 
 * USER_REGION
 *      This is the dedicated space 0...7FFFFFFFFFFF for userspace. 
 *      No kernel objects or data will be put into this space.
 *      Protocol 1 puts the page tables at 0xA000 by default, so these will have to be moved
 *       up to kernel space.
 * 
 * KERNEL_STACK_REGION
 * KERNEL_STACK_END
 *      Encapsulate a 1GB large area of memory, to be used by the kernel for thread & interrupt stacks,
 *       call unwinding and other debug information.
 * 
 * KERNEL_HEAP_REGION
 * KERNEL_HEAP_END
 *      Encapsulate another 1GB large area for kernel objects. ie. resources (images, sounds), libraries,
 *       data structures, assorted information about the system.. etc.
 * 
 * DIRECT_REGION
 *      As mentioned above, the lower half is reserved for user space.
 *      The higher half will be direct-mapped throughout.
 *      This is the cutoff for the higher half - FFFF800000000000.
 *      
 */

#define MMIO_REGION         0xFFFFFFFFF8000000ull // Cannot move!
#define FB_REGION           0xFFFFFFFFFC000000ull // Cannot move!
#define FB_PHYSICAL         0x00000000FD000000ull // Physical location of the Framebuffer
#define KERNEL_REGION       0xFFFFFFFFFFE00000ull // -2MiB, from bootloader

#define USER_REGION         0x00007FFFFFFFFFFFull // Not needed yet, but we're higher half so we might as well be thorough

#define KERNEL_STACK_REGION 0xFFFFE00000000000ull  // Kernel Stack Space
#define KERNEL_STACK_END    0xFFFFE00040000000ull  // End of Kernel Stack Space

#define KERNEL_HEAP_REGION  0xFFFFE00080000000ull  // Kernel Object Space (kmalloc will allocate into this region)
#define KERNEL_HEAP_END     0xFFFFE000C0000000ull  // End of Kernel Object Space

#define DIRECT_REGION       0xFFFF800000000000ull

#define LOWER_REGION        0x0000000100000000ull  // Lower Memory cutoff - 4GB

#define PAGE_SHIFT 12

/*********************************************
*      T Y P E   D E F I N I T I O N S
**********************************************/

typedef void* directptr_t;

typedef struct {
    ticketlock_t Lock;

    directptr_t  PML4;
} address_space_t;

typedef enum {
    MAP_WRITE = 0x1,
    MAP_EXEC = 0x2,
} mapflags_t;

typedef enum {
    CACHE_NONE,
    CACHE_WRITE_THROUGH,
    CACHE_WRITE_BACK,
    CACHE_WRITE_COMBINING
} pagecache_t;

typedef struct {
    int MaxOrder;

    directptr_t Base;

    directptr_t* List;

    ticketlock_t Lock;
} buddy_t;

/*********************************************
*    A b s t r a c t     A l l o c a t o r
**********************************************/

const char* IntToAscii(int In);

typedef void* allocator_t;
typedef void* mempool_t;

allocator_t CreateAllocator(void* Memory);
allocator_t CreateAllocatorWithPool(void* Memory, size_t Bytes);

void        DestroyAllocator(allocator_t Allocator);

mempool_t   GetPoolFromAllocator(allocator_t Allocator);

mempool_t   AddPoolToAllocator(allocator_t Allocator, void* Memory, size_t Bytes);
void        RemovePoolFromAllocator(allocator_t Allocator, mempool_t pool);

void*       AllocatorMalloc (allocator_t Allocator, size_t Bytes);
void*       AllocatorMalign (allocator_t Allocator, size_t Alignment, size_t Bytes);
void*       AllocatorRealloc(allocator_t Allocator, void* VirtualAddress, size_t NewSize);
void        AllocatorFree   (allocator_t Allocator, void* VirtualAddress);

size_t      AllocatorGetBlockSize(void* VirtualAddress);

size_t      AllocatorSize(void);
size_t      AllocatorAlignSize(void);
size_t      AllocatorMinBlockSize(void);
size_t      AllocatorMaxBlockSize(void);

size_t      AllocatorPoolOverhead(void);
size_t      AllocatorAllocateOverhead(void);


size_t AlignUpwards(size_t Pointer, size_t Alignment);
size_t AlignDownwards(size_t Pointer, size_t Alignment);
void* AlignPointer(const void* Pointer, size_t Alignment);


/************************************************************
*      C h r o m a    M e m o r y   M a n a g e m e n t
*************************************************************/

extern size_t memstart;

extern size_t end;

void        ListMemoryMap();

void        InitMemoryManager();

void        AddRangeToPhysMem(directptr_t Base, size_t Size);

directptr_t PhysAllocateLowMem(size_t Size);

directptr_t PhysAllocateMem(size_t Size);

directptr_t PhysAllocateZeroMem(size_t Size);

directptr_t PhysAllocateLowZeroMem(size_t Size);

directptr_t PhysAllocatePage();

void        PhysRefPage(directptr_t Page);

void        PhysFreePage(directptr_t Page);

void        FreePhysMem(directptr_t Phys);

size_t      SeekFrame();

void        MemoryTest();

void        InitPaging();

void        TraversePageTables();

void*       memcpy(void* dest, void const* src, size_t len);


/*********************************************
*      C h r o m a     A l l o c a t o r
**********************************************/

void  SetAddressSpace(address_space_t* Space);
//TODO: Copy to/from Userspace
void  MapVirtualMemory(address_space_t* Space, void* VirtualAddress, size_t PhysicalAddress, mapflags_t Flags);
void  UnmapVirtualMemory(address_space_t* Space, void* VirtualAddress);

void  CacheVirtualMemory(address_space_t* Space, void* VirtualAddress, pagecache_t CacheType);

void* AllocateMemory(size_t Bits);

void* ReallocateMemory(void* VirtualAddress, size_t NewSize);

void  FreeMemory(void* VirtualAddress);

void* AllocateKernelStack();

void  FreeKernelStack(void* StackAddress);

void  PageFaultHandler(INTERRUPT_FRAME Frame);