/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/


#pragma once

#include<stddef.h>
#include<stdint.h>
#include <lainlib/mutex/spinlock.h>

struct vector_t {
    void** items;
    size_t n;

    spinlock_t vector_lock;
};

int VectorRemoveItem(struct vector_t* vec, void* item);

int VectorRemove(struct vector_t* vec, size_t index);

void* VectorGet(struct vector_t* vec, size_t index);

int VectorInsert(struct vector_t* vec, void* item, size_t index);

int VectorAppend(struct vector_t* vec, void* item);

int VectorDestroy(struct vector_t* vec);