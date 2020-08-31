
#pragma once

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/


/* Defines all of the temporary library functions.
 * All of this must be moved into the Chroma stdlib.
 * They exist here as guidance, and as utility for the kernel itself.
 * If need be, they can also be moved into a trimmed-down "kernel libc" or "libk".
 */



#include <lainlib/vector/vector.h>

#include <lainlib/list/list.h>

#include <lainlib/mutex/spinlock.h>
#include <lainlib/mutex/ticketlock.h>

#include <lainlib/compression/lzg.h>
