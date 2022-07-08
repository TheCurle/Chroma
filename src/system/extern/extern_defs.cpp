/************************
 *** Team Kitty, 2022 ***
 ***     Chroma       ***
 ***********************/

#include <kernel/system/descriptors.h>

/**
 * Contains definitions that are used by external files.
 * That means, independent programs or assembler files.
 */

/**
 * The template GDT entry for a newly initialized core.
 * Protected Mode.
 * TODO: Document what the entries here are.
 */

__attribute__((aligned(64))) size_t ProtectedGDTEntry[3] = {
        0,
        0x00CF9A000000FFFF,
        0x00CF92000000FFFF
};

/**
 * The GDT table value to be loaded into each newly initialized core.
 * Protected Mode.
 */

DESC_TBL ProtectedGDT = {
        .Limit = sizeof(ProtectedGDTEntry) - 1,
        .Base = (size_t) &ProtectedGDTEntry
};

/**
 * The template GDT entry for a newly initialized core.
 * Long Mode.
 * TODO: Document what the entries here are.
 */
__attribute__((aligned(64))) size_t LongGDTEntry[3] = {
        0,
        0x00AF98000000FFFF,
        0x00CF92000000FFFF
};

/**
 * The GDT table value to be loaded into each newly initialized core.
 * Long Mode.
 */

DESC_TBL LongGDT = {
        .Limit = sizeof(LongGDTEntry) - 1,
        .Base = (size_t) &LongGDTEntry
};

