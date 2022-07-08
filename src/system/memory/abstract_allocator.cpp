#include <stddef.h>
#include <kernel/system/memory.h>
#include <kernel/system/io.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************
*    C O N S T A N T S   A N D   M A C R O S
*************************************************/

#define BLOCK_FREE      (1 << 0)
#define BLOCK_PREV_FREE (1 << 1)

#define BLOCK_OVERHEAD  (sizeof(size_t))

#define BLOCK_OFFSET    (offsetof(block_header_t, Size) + sizeof(size_t))

#define BLOCK_MIN_SIZE  (sizeof(block_header_t) - sizeof(block_header_t*))
#define BLOCK_MAX_SIZE  (CAST(size_t, 1) << FL_LIMIT)

#define static_assert _Static_assert

extern void SomethingWentWrong(const char* Message);

//#define ASSERT(X) _Static_assert(X)
/************************************************
*         S A N I T Y   C H E C K S
*************************************************/

//_Static_Assert(sizeof(int) * __CHAR_BIT__ == 32);
//_Static_Assert(sizeof(int) * __CHAR_BIT__ == 32);
//_Static_Assert(sizeof(size_t) * __CHAR_BIT__ >= 32);
//_Static_Assert(sizeof(size_t) * __CHAR_BIT__ <= 64);
//_Static_Assert(sizeof(unsigned int) * __CHAR_BIT__ >= SL_INDEX_COUNT);
//_Static_Assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);


/************************************************
*           F F S   A N D   F L S
*************************************************/

#ifdef _cplusplus
  #define alloc_decl inline
#else
  #define alloc_decl static
#endif


alloc_decl int Alloc_FindFirstOne(unsigned int word) {
    return __builtin_ffs(word) - 1;
}

alloc_decl int Alloc_FindLastOne(unsigned int word) {
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

alloc_decl int Alloc_FindLastOne_64(size_t size) {

    int high = (int) (size >> 32);
    int bits = 0;

    if (high)
        bits = 32 + Alloc_FindLastOne(high);
    else
        bits = Alloc_FindLastOne((int) size & 0xFFFFFFFF);

    return bits;
}

#undef alloc_decl


/*********************************************
*      T Y P E   D E F I N I T I O N S
**********************************************/

enum Alloc_Public {

    SL_LIMIT_LN = 5,
};

enum Alloc_Private {
    ALIGN_SIZE_LN = 3,
    ALIGN_SIZE = (1 << ALIGN_SIZE_LN),

    FL_LIMIT = 32,

    SL_INDEX_COUNT = (1 << SL_LIMIT_LN),

    FL_INDEX_SHIFT = (SL_LIMIT_LN + ALIGN_SIZE_LN),
    FL_INDEX_COUNT = (FL_LIMIT - FL_INDEX_SHIFT + 1),

    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),

};


typedef struct block_header_t {
    struct block_header_t* LastBlock;

    size_t Size; // Not including this header

    struct block_header_t* NextFreeBlock;
    struct block_header_t* LastFreeBlock;
} block_header_t;


typedef struct allocator_control_t {

    block_header_t BlockNull;

    unsigned int FirstLevel_Bitmap;
    unsigned int SecondLevel_Bitmap[FL_INDEX_COUNT];

    block_header_t* Blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
} allocator_control_t;


/**********************************************************************************
*      B L O C K _ H E A D E R _ T      M E M B E R     F U N C T I O N S
************************************************************************************/

static size_t BlockSize(const block_header_t* Block) {
    return Block->Size & ~(BLOCK_FREE | BLOCK_PREV_FREE);
}

static void BlockSetSize(block_header_t* Block, size_t Size) {
    Block->Size = Size | (Block->Size & (BLOCK_FREE | BLOCK_PREV_FREE));
}

static int BlockIsLast(const block_header_t* Block) {
    return BlockSize(Block) == 0;
}

static int BlockIsFree(const block_header_t* Block) {
    return CAST(int, Block->Size & BLOCK_FREE);
}

static void BlockSetFree(block_header_t* Block) {
    Block->Size |= BLOCK_FREE;
}

static void BlockSetUsed(block_header_t* Block) {
    Block->Size &= ~BLOCK_FREE;
}

static int BlockPrevIsFree(const block_header_t* Block) {
    return CAST(int, Block->Size & BLOCK_PREV_FREE);
}

static void BlockSetPrevFree(block_header_t* Block) {
    Block->Size |= BLOCK_PREV_FREE;
}

static void BlockSetPrevUsed(block_header_t* Block) {
    Block->Size &= ~BLOCK_PREV_FREE;
}

static block_header_t* WhichBlock(const void* Address) {
    return CAST(block_header_t*, CAST(unsigned char*, Address) - BLOCK_OFFSET);
}

static void* WhereBlock(const block_header_t* Block) {
    return CAST(void*, CAST(unsigned char*, Block) + BLOCK_OFFSET);
}

static block_header_t* OffsetToBlock(const void* Address, size_t Size) {
    return CAST(block_header_t*, CAST(ptrdiff_t, Address) + Size);
}

static block_header_t* BlockGetPrevious(const block_header_t* Current) {
    ASSERT(BlockPrevIsFree(Current), "BlockGetPrevious: Previous block NOT free");
    return Current->LastBlock;
}

static block_header_t* BlockGetNext(const block_header_t* Current) {
    block_header_t* NextBlock = OffsetToBlock(WhereBlock(Current), BlockSize(Current) - BLOCK_OVERHEAD);
    ASSERT(!BlockIsLast(Current), "BlockGetNext: Current block is last!");
    return NextBlock;
}

static block_header_t* BlockLinkToNext(block_header_t* Current) {
    block_header_t* NextBlock = BlockGetNext(Current);
    NextBlock->LastBlock = Current;
    return NextBlock;
}

static void BlockMarkFree(block_header_t* Current) {
    block_header_t* NextBlock = BlockLinkToNext(Current);
    BlockSetPrevFree(NextBlock);
    BlockSetFree(Current);
}

static void BlockMarkUsed(block_header_t* Current) {
    block_header_t* NextBlock = BlockGetNext(Current);
    BlockSetPrevUsed(NextBlock);
    BlockSetUsed(Current);
}


/***********************************************************************************
*            P O I N T E R       A L I G N M E N T       F U N C T I O N S
************************************************************************************/

size_t AlignUpwards(size_t Pointer, size_t Alignment) {
    //ASSERT(((Alignment & (Alignment - 1)) == 0));
    return (Pointer + (Alignment - 1)) & ~(Alignment - 1);
}

size_t AlignDownwards(size_t Pointer, size_t Alignment) {
    //ASSERT((Alignment & (Alignment - 1) == 0));
    return (Pointer - (Pointer & (Alignment - 1)));
}

void* AlignPointer(const void* Pointer, size_t Alignment) {

    const ptrdiff_t AlignedPointer =
            ((
                     CAST(ptrdiff_t, Pointer)
                     + (Alignment - 1))
             & ~(Alignment - 1)
            );
    ASSERT(((Alignment & (Alignment - 1)) == 0), "AlignPointer: Requested alignment not aligned!");

    return CAST(void*, AlignedPointer);
}


/***********************************************************************************
*            M E M O R Y        B L O C K         M A N A G E M E N T
************************************************************************************/

static size_t AlignRequestSize(size_t Size, size_t Alignment) {
    size_t Adjustment = 0;

    if (Size) {
        const size_t Aligned = AlignUpwards(Size, Alignment);

        if (Aligned < BLOCK_MAX_SIZE)
            Adjustment = MAX(Aligned, BLOCK_MIN_SIZE);

    }

    return Adjustment;
}

static void InsertMapping(size_t Size, int* FirstLevelIndex, int* SecondLevelIndex) {
    int FirstLevel, SecondLevel;

    if (Size < SMALL_BLOCK_SIZE) {

        FirstLevel = 0;
        SecondLevel = CAST(int, Size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    } else {
        FirstLevel = Alloc_FindLastOne_64(Size);
        SecondLevel = CAST(int, Size >> (FirstLevel - SL_LIMIT_LN)) ^ (1 << SL_LIMIT_LN);

        FirstLevel -= (FL_INDEX_SHIFT - 1);
    }

    *FirstLevelIndex = FirstLevel;
    *SecondLevelIndex = SecondLevel;
}

static void RoundUpBlockSize(size_t Size, int* FirstLevelIndex, int* SecondLevelIndex) {
    if (Size >= SMALL_BLOCK_SIZE) {
        const size_t Rounded = (1 << (Alloc_FindLastOne_64(Size) - SL_LIMIT_LN)) - 1;
        Size += Rounded;
    }

    InsertMapping(Size, FirstLevelIndex, SecondLevelIndex);
}

static block_header_t* FindSuitableBlock(allocator_control_t* Controller, int* FirstLevelIndex, int* SecondLevelIndex) {
    int FirstLevel = *FirstLevelIndex;
    int SecondLevel = *SecondLevelIndex;

    unsigned int SLMap = Controller->SecondLevel_Bitmap[FirstLevel] & (~0U << SecondLevel);

    if (!SLMap) {

        const unsigned int FLMap = Controller->FirstLevel_Bitmap & (~0U << (FirstLevel + 1));

        if (!FLMap)
            return 0;

        FirstLevel = Alloc_FindFirstOne(FLMap);
        *FirstLevelIndex = FirstLevel;
        SLMap = Controller->SecondLevel_Bitmap[FirstLevel];
    }

    ASSERT(SLMap, "FindSuitableBlock: Second level bitmap not present!");

    SecondLevel = Alloc_FindFirstOne(SLMap);
    *SecondLevelIndex = SecondLevel;

    return Controller->Blocks[FirstLevel][SecondLevel];
}

static void RemoveFreeBlock(allocator_control_t* Controller, block_header_t* Block, int FirstLevel, int SecondLevel) {
    block_header_t* PreviousBlock = Block->LastFreeBlock;
    block_header_t* NextBlock = Block->NextFreeBlock;

    ASSERT(PreviousBlock, "RemoveFreeBlock: PreviousBlock is null!");
    ASSERT(NextBlock, "RemoveFreeBlock: NextBlock is null!");

    NextBlock->LastFreeBlock = PreviousBlock;
    PreviousBlock->NextFreeBlock = NextBlock;

    if (Controller->Blocks[FirstLevel][SecondLevel] == Block) {
        Controller->Blocks[FirstLevel][SecondLevel] = NextBlock;

        if (NextBlock == &Controller->BlockNull) {
            Controller->SecondLevel_Bitmap[FirstLevel] &= ~(1U << SecondLevel);

            if (!Controller->SecondLevel_Bitmap[FirstLevel]) {
                Controller->FirstLevel_Bitmap &= ~(1U << FirstLevel);
            }
        }
    }
}

static void
InsertFreeBlock(allocator_control_t* Controller, block_header_t* NewBlock, int FirstLevel, int SecondLevel) {
    block_header_t* Current = Controller->Blocks[FirstLevel][SecondLevel];

    ASSERT(Current, "InsertFreeBlock: Current Block is null!");
    if (!Current) {
        SerialPrintf(
                "Extra info: \r\n\tFirst Level: %x Second Level: %x\r\nFirst Level bitmap: %x, Second Level bitmap: %x\r\n\tBlocks %x, BlocksAddress: %x",
                FirstLevel, SecondLevel, Controller->FirstLevel_Bitmap, Controller->SecondLevel_Bitmap,
                Controller->Blocks, Controller->Blocks[FirstLevel][SecondLevel]);
        for (;;) { }
    }
    ASSERT(NewBlock, "InsertFreeBlock: New Block is null!");

    NewBlock->NextFreeBlock = Current;
    NewBlock->LastFreeBlock = &Controller->BlockNull;

    Current->LastFreeBlock = NewBlock;

    ASSERT(WhereBlock(NewBlock) == AlignPointer(WhereBlock(NewBlock), ALIGN_SIZE),
           "InsertFreeBlock: Current block is not memory aligned!");

    Controller->Blocks[FirstLevel][SecondLevel] = NewBlock;
    Controller->FirstLevel_Bitmap |= (1U << FirstLevel);
    Controller->SecondLevel_Bitmap[FirstLevel] |= (1U << SecondLevel);

}

static void RemoveBlock(allocator_control_t* Controller, block_header_t* Block) {
    int FirstLevel, SecondLevel;

    InsertMapping(BlockSize(Block), &FirstLevel, &SecondLevel);
    RemoveFreeBlock(Controller, Block, FirstLevel, SecondLevel);
}

static void InsertBlock(allocator_control_t* Controller, block_header_t* Block) {
    int FirstLevel, SecondLevel;
    InsertMapping(BlockSize(Block), &FirstLevel, &SecondLevel);
    InsertFreeBlock(Controller, Block, FirstLevel, SecondLevel);
}

static int CanBlockSplit(block_header_t* Block, size_t NewSize) {
    return BlockSize(Block) >= sizeof(block_header_t) + NewSize;
}

static block_header_t* SplitBlock(block_header_t* Block, size_t NewSize) {
    block_header_t* Overlap = OffsetToBlock(WhereBlock(Block), NewSize - BLOCK_OVERHEAD);

    const size_t RemainingSize = BlockSize(Block) - (NewSize + BLOCK_OVERHEAD);

    ASSERT(WhereBlock(Overlap) == AlignPointer(WhereBlock(Overlap), ALIGN_SIZE),
           "SplitBlock: Requested size results in intermediary block which is not aligned!");

    ASSERT(BlockSize(Block) == RemainingSize + NewSize + BLOCK_OVERHEAD, "SplitBlock: Maths error!");

    BlockSetSize(Overlap, RemainingSize);

    ASSERT(BlockSize(Overlap) >= BLOCK_MIN_SIZE, "SplitBlock: Requested size results in new block that is too small!");

    BlockSetSize(Block, NewSize);

    BlockMarkFree(Overlap);

    return Overlap;
}

static block_header_t* MergeBlockDown(block_header_t* Previous, block_header_t* Block) {
    ASSERT(!BlockIsLast(Previous), "MergeBlockDown: Previous block is the last block! (Current block is first block?)");

    Previous->Size += BlockSize(Block) + BLOCK_OVERHEAD;
    BlockLinkToNext(Previous);
    return Previous;
}

static block_header_t* MergeEmptyBlockDown(allocator_control_t* Controller, block_header_t* Block) {

    if (BlockPrevIsFree(Block)) {
        block_header_t* Previous = BlockGetPrevious(Block);
        ASSERT(Previous, "MergeEmptyBlockDown: Previous block is null!");
        ASSERT(BlockIsFree(Previous), "MergeEmptyBlockDown: Previous block is free!");
        RemoveBlock(Controller, Previous);
        Block = MergeBlockDown(Previous, Block);
    }

    return Block;
}

static block_header_t* MergeNextBlockDown(allocator_control_t* Controller, block_header_t* Block) {
    block_header_t* NextBlock = BlockGetNext(Block);
    ASSERT(NextBlock, "MergeNextBlockDown: Next Block is null!");

    if (BlockIsFree(NextBlock)) {
        ASSERT(!BlockIsLast(Block), "MergeNextBlockDown: Current block is the last block!");
        RemoveBlock(Controller, NextBlock);
        Block = MergeBlockDown(Block, NextBlock);
    }

    return Block;
}

static void TrimBlockFree(allocator_control_t* Controller, block_header_t* Block, size_t Size) {
    ASSERT(BlockIsFree(Block), "TrimBlockFree: Current block is wholly free!");

    if (CanBlockSplit(Block, Size)) {
        block_header_t* RemainingBlock = SplitBlock(Block, Size);

        BlockLinkToNext(Block);

        BlockSetPrevFree(RemainingBlock);

        InsertBlock(Controller, RemainingBlock);
    }
}

static void TrimBlockUsed(allocator_control_t* Controller, block_header_t* Block, size_t Size) {
    ASSERT(!BlockIsFree(Block), "TrimBlockUsed: The current block is wholly used!");

    if (CanBlockSplit(Block, Size)) {

        block_header_t* RemainingBlock = SplitBlock(Block, Size);

        BlockSetPrevUsed(RemainingBlock);

        RemainingBlock = MergeNextBlockDown(Controller, RemainingBlock);

        InsertBlock(Controller, RemainingBlock);
    }
}


static block_header_t* TrimBlockLeadingFree(allocator_control_t* Controller, block_header_t* Block, size_t Size) {
    block_header_t* RemainingBlock = Block;

    if (CanBlockSplit(Block, Size)) {
        RemainingBlock = SplitBlock(Block, Size - BLOCK_OVERHEAD);

        BlockSetPrevFree(RemainingBlock);

        BlockLinkToNext(Block);
        InsertBlock(Controller, Block);
    }

    return RemainingBlock;
}

static block_header_t* LocateFreeBlock(allocator_control_t* Controller, size_t Size) {

    int FirstLevel = 0, SecondLevel = 0;

    block_header_t* Block = 0;

    if (Size) {

        RoundUpBlockSize(Size, &FirstLevel, &SecondLevel);

        if (FirstLevel < FL_INDEX_COUNT) {
            Block = FindSuitableBlock(Controller, &FirstLevel, &SecondLevel);
        }

    }

    if (Block) {
        ASSERT(BlockSize(Block) >= Size, "LocateFreeBlock: Found a block that is too small!");
        RemoveFreeBlock(Controller, Block, FirstLevel, SecondLevel);
    }

    return Block;
}

static void* PrepareUsedBlock(allocator_control_t* Controller, block_header_t* Block, size_t Size) {
    void* Pointer = 0;

    if (Block) {
        ASSERT(Size, "PrepareUsedBlock: Size is 0!");
        TrimBlockFree(Controller, Block, Size);
        BlockMarkUsed(Block);
        Pointer = WhereBlock(Block);
    }

    return Pointer;
}


/***********************************************************************************
*              C O N T R O L L E R         M A N A G E M E N T
************************************************************************************/

static void ConstructController(allocator_control_t* Controller) {
    int i, j;

    Controller->BlockNull.NextFreeBlock = &Controller->BlockNull;
    Controller->BlockNull.LastFreeBlock = &Controller->BlockNull;

    Controller->FirstLevel_Bitmap = 0;

    for (i = 0; i < FL_INDEX_COUNT; i++) {
        Controller->SecondLevel_Bitmap[i] = 0;

        for (j = 0; j < SL_INDEX_COUNT; j++) {
            Controller->Blocks[i][j] = &Controller->BlockNull;
        }
    }
}


/***********************************************************************************
*               H E A D E R  ( A P I )         F U N C T I O N S
************************************************************************************/


size_t AllocatorGetBlockSize(void* Memory) {
    size_t Size = 0;

    if (Memory) {
        const block_header_t* Block = WhichBlock(Memory);
        Size = BlockSize(Block);
    }

    return Size;
}

size_t AllocatorSize(void) {
    return sizeof(allocator_control_t);
}

size_t AllocatorAlignSize(void) {
    return ALIGN_SIZE;
}

size_t AllocatorMinBlockSize(void) {
    return BLOCK_MIN_SIZE;
}

size_t AllocatorMaxBlockSize(void) {
    return BLOCK_MAX_SIZE;
}

size_t AllocatorPoolOverhead(void) {
    return 2 * BLOCK_OVERHEAD; // Free block + Sentinel block
}

size_t AllocatorAllocateOverhead(void) {
    return BLOCK_OVERHEAD;
}

mempool_t AddPoolToAllocator(allocator_t Allocator, void* Address, size_t Size) {

    block_header_t* Block;
    block_header_t* NextBlock;

    const size_t PoolOverhead = AllocatorPoolOverhead();
    const size_t PoolBytes = AlignDownwards(Size - PoolOverhead, ALIGN_SIZE);

    if (((ptrdiff_t) Address % ALIGN_SIZE) != 0) {
        SerialPrintf("Memory manager error at [%s:%x]: Memory not properly aligned.\r\n", __FILE__, __LINE__);
        return 0;
    }

    if (PoolBytes < BLOCK_MIN_SIZE || PoolBytes > BLOCK_MAX_SIZE) {
        SerialPrintf("Memory manager error at [%s:%x]: Memory Size out of bounds: 0x%x-0x%x: 0x%x.\r\n", __FILE__,
                     __LINE__, (unsigned int) (PoolOverhead + BLOCK_MIN_SIZE),
                     (unsigned int) (PoolOverhead + BLOCK_MAX_SIZE) / 256, PoolBytes);
        return 0;
    }

    Block = OffsetToBlock(Address, -(ptrdiff_t) BLOCK_OVERHEAD);
    BlockSetSize(Block, PoolBytes);
    BlockSetFree(Block);
    BlockSetPrevUsed(Block);

    InsertBlock(CAST(allocator_control_t*, Allocator), Block);

    NextBlock = BlockLinkToNext(Block);
    BlockSetSize(NextBlock, 0);
    BlockSetUsed(NextBlock);
    BlockSetPrevFree(NextBlock);

    return Address;
}

void RemovePoolFromAllocator(allocator_t Allocator, mempool_t Pool) {
    allocator_control_t* Controller = CAST(allocator_control_t*, Allocator);
    block_header_t* Block = OffsetToBlock(Pool, -(int) BLOCK_OVERHEAD);

    int FirstLevel = 0, SecondLevel = 0;

    ASSERT(BlockIsFree(Block), "RemovePoolFromAllocator: Current block is free!");
    ASSERT(!BlockIsFree(BlockGetNext(Block)), "RemovePoolFromAllocator: Next Block is not free!");
    ASSERT(BlockSize(BlockGetNext(Block)) == 0, "RemovePoolFromAllocator: Next block is size 0!");

    RoundUpBlockSize(BlockSize(Block), &FirstLevel, &SecondLevel);
    RemoveFreeBlock(Controller, Block, FirstLevel, SecondLevel);
}

int TestBuiltins() {
    /* Verify ffs/fls work properly. */
    int TestsFailed = 0;
    TestsFailed += (Alloc_FindFirstOne(0) == -1) ? 0 : 0x1;
    TestsFailed += (Alloc_FindLastOne(0) == -1) ? 0 : 0x2;
    TestsFailed += (Alloc_FindFirstOne(1) == 0) ? 0 : 0x4;
    TestsFailed += (Alloc_FindLastOne(1) == 0) ? 0 : 0x8;
    TestsFailed += (Alloc_FindFirstOne(0x80000000) == 31) ? 0 : 0x10;
    TestsFailed += (Alloc_FindFirstOne(0x80008000) == 15) ? 0 : 0x20;
    TestsFailed += (Alloc_FindLastOne(0x80000008) == 31) ? 0 : 0x40;
    TestsFailed += (Alloc_FindLastOne(0x7FFFFFFF) == 30) ? 0 : 0x80;

    TestsFailed += (Alloc_FindLastOne_64(0x80000000) == 31) ? 0 : 0x100;
    TestsFailed += (Alloc_FindLastOne_64(0x100000000) == 32) ? 0 : 0x200;
    TestsFailed += (Alloc_FindLastOne_64(0xffffffffffffffff) == 63) ? 0 : 0x400;

    if (TestsFailed) {
        SerialPrintf("TestBuiltins: %x ffs/fls tests failed.\n", TestsFailed);
    }

    return TestsFailed;
}

allocator_t CreateAllocator(void* Memory) {
    if (TestBuiltins())
        return 0;

    if (((ptrdiff_t) Memory % ALIGN_SIZE) != 0) {
        SerialPrintf("Memory manager error at [%s:%x]: Memory not properly aligned.\r\n", __FILE__, __LINE__);
        return 0;
    }

    ConstructController(CAST(allocator_control_t*, Memory));

    return CAST(allocator_t, Memory);
}


allocator_t CreateAllocatorWithPool(void* Memory, size_t Bytes) {
    allocator_t Allocator = CreateAllocator(Memory);

    AddPoolToAllocator(Allocator, (char*) Memory + AllocatorSize(), Bytes - AllocatorSize());

    return Allocator;
}

void DestroyAllocator(allocator_t Allocator) {
    (void) Allocator;
}

mempool_t GetPoolFromAllocator(allocator_t Allocator) {
    return CAST(mempool_t, (char*) Allocator + AllocatorSize());
}

/***********************************************************************************
*             S T D L I B         A L L O C A T E          F U N C T I O N S
************************************************************************************/

void* AllocatorMalloc(allocator_t Allocator, size_t Size) {
    allocator_control_t* Controller = CAST(allocator_control_t*, Allocator);
    const size_t Adjustment = AlignRequestSize(Size, ALIGN_SIZE);
    block_header_t* Block = LocateFreeBlock(Controller, Adjustment);
    return PrepareUsedBlock(Controller, Block, Adjustment);
}

void* AllocatorMalign(allocator_t Allocator, size_t Alignment, size_t Size) {
    allocator_control_t* Controller = CAST(allocator_control_t*, Allocator);
    const size_t Adjustment = AlignRequestSize(Size, ALIGN_SIZE);


    const size_t MinimumGap = sizeof(block_header_t);

    const size_t SizeWithGap = AlignRequestSize(Adjustment + Alignment + MinimumGap, Alignment);

    const size_t AlignedSize = (Adjustment && Alignment > ALIGN_SIZE) ? SizeWithGap : Adjustment;

    block_header_t* Block = LocateFreeBlock(Controller, AlignedSize);

    ASSERT(sizeof(block_header_t) == BLOCK_MIN_SIZE + BLOCK_OVERHEAD, "AllocatorMalign: Maths error!");

    if (Block) {
        void* Address = WhereBlock(Block);
        void* AlignedAddress = AlignPointer(Address, Alignment);

        size_t Gap = CAST(size_t, CAST(ptrdiff_t, AlignedAddress) - CAST(ptrdiff_t, Address));

        if (Gap) {
            if (Gap << MinimumGap) {
                const size_t GapRemaining = MinimumGap - Gap;
                const size_t Offset = MAX(GapRemaining, Alignment);
                const void* NextAlignedAddress = CAST(void*, CAST(ptrdiff_t, AlignedAddress) + Offset);

                AlignedAddress = AlignPointer(NextAlignedAddress, Alignment);

                Gap = CAST(size_t, CAST(ptrdiff_t, AlignedAddress) - CAST(ptrdiff_t, Address));
            }

            ASSERT(Gap >= MinimumGap, "AllocatorMalign: Maths error 2!");

            Block = TrimBlockLeadingFree(Controller, Block, Gap);

        }
    }

    return PrepareUsedBlock(Controller, Block, Adjustment);
}

void AllocatorFree(allocator_t Allocator, void* Address) {
    if (Address) {
        allocator_control_t* Controller = CAST(allocator_control_t*, Allocator);
        block_header_t* Block = WhichBlock(Address);
        ASSERT(!BlockIsFree(Block), "AllocatorFree: Attempting to free a freed block!");

        BlockMarkFree(Block);
        Block = MergeEmptyBlockDown(Controller, Block);
        Block = MergeNextBlockDown(Controller, Block);

        InsertBlock(Controller, Block);
    }
}

/*
 * Realloc should, with:
 *   * A valid size with an invalid pointer:
 *      - Allocate space
 *   * An invalid size with a valid pointer:
 *      - Free Space
 *   * An invalid request:
 *      - Do nothing
 *   * A valid extension request:
 *      - Leave the new area as it is
 *      // TODO: memset this area to 0.
 */


void* AllocatorRealloc(allocator_t Allocator, void* Address, size_t NewSize) {
    allocator_control_t* Controller = CAST(allocator_control_t*, Allocator);

    void* Pointer = 0;

    // Valid address, invalid size; free
    if (Address && NewSize == 0)
        AllocatorFree(Allocator, Address);

    else if (!Address)  // Invalid address; alloc
        AllocatorMalloc(Allocator, NewSize);

    else {
        block_header_t* Block = WhichBlock(Address);
        block_header_t* NextBlock = BlockGetNext(Block);

        const size_t CurrentSize = BlockSize(Block);
        const size_t CombinedSize = CurrentSize + BlockSize(NextBlock) + BLOCK_OVERHEAD;

        const size_t AdjustedSize = AlignRequestSize(NewSize, ALIGN_SIZE);

        ASSERT(!BlockIsFree(Block), "AllocatorRealloc: Requested block is not free!");

        if (AdjustedSize > CurrentSize && (!BlockIsFree(NextBlock) || AdjustedSize > CombinedSize)) {
            // We're going to need more room

            Pointer = AllocatorMalloc(Allocator, NewSize);
            if (Pointer) {
                const size_t MinimumSize = MIN(CurrentSize, NewSize);
                memcpy(Pointer, Address, MinimumSize);
                AllocatorFree(Allocator, Address);
            }
        } else {
            if (AdjustedSize > CurrentSize) {
                MergeNextBlockDown(Controller, Block);
                BlockMarkUsed(Block);
            }

            TrimBlockUsed(Controller, Block, AdjustedSize);
            Pointer = Address;
        }
    }

    return Pointer;
}

#ifdef  __cplusplus
}
#endif