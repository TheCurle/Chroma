#pragma once

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/


typedef volatile int spinlock_t;

/* A set of macros that acquire and release a mutex spinlock. */
//TODO: this *needs* to be moved to a kernel header.

#define SPINLOCK(name) \
    while( !__sync_bool_compare_and_swap(name, 0, 1)); \
    __sync_synchronize();


#define SPUNLOCK(name) \
    __sync_synchronize(); \
    name = 0;

