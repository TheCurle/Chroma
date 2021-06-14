#include <templib/templib.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file provides a Chroma implementation of std::vector, a C++ standard library class.
 * It has a lot of work left to be done, but it's usable for its intended function (Graphics)
 */

int VectorRemoveItem(struct vector_t* vec, void* item) {
    //TODO
    return -1;

}

int VectorRemove(struct vector_t* vec, size_t index) {

    if (!vec) return 0;

    //TODO: Vector spinlock
    // AcqSpinlock(&vec->lock);

    if((index + 1) > vec->n) return 0;

    vec->items[index] = NULL;

    for (int i = 0; i < vec->n; i++) {
        vec->items[i] = vec->items[i + 1];
    }
    
    //TODO: vector reallocate
    // realloc(vec->items, vec->n - 1);

    vec->n--;

    // ReleaseSpinlock(&vec->lock);

    return 1;

}

void* VectorGet(struct vector_t* vec, size_t index) {

    if(!vec) return 0;

    //TODO: Vector spinlock
    // AcqSpinlock(&vec->lock);

    if((index + 1) > vec->n) return NULL;

    // ReleaseSpinlock(&vec->lock);
    return vec->items[index];

}

int VectorInsert(struct vector_t* vec, void* item, size_t index) {

}

int VectorAppend(struct vector_t* vec, void* item) {

}

int VectorDestroy(struct vector_t* vec) {

}