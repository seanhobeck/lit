/**
*	@author Sean Hobeck
 *	@date 2025-11-12
 */
#ifndef DYNA_H
#define DYNA_H

/*! @uses size_t */
#include <stddef.h>

/// @note a structure for a dynamic array.
typedef struct {
    // array of data.
    void** data;
    // length (count) and capacity of the dynamic array.
    size_t length, capacity, isize;
    // element size; sizeof the data in the array.
} dyna_t;

/**
 * @brief create a dyna_t structure with a set item size.
 *
 * @param isize item size.
 * @return an allocated dynamic array.
 */
dyna_t*
dyna_create(size_t isize);

/**
 * @brief destroying / freeing a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 */
void
dyna_free(dyna_t* array);

/**
 * @brief push new item/data onto a dynamically allocated array.
 *
 * @param array pointer to a dynamically allocated array.
 * @param data data to be pushed onto the top of the allocated array.
 */
void
dyna_push(dyna_t* array, void* data);

/**
 * @brief pop data out of a dynamically allocated array,
 *  while also making sure to shift down / coalesce the memory.
 *  this function does not SHRINK the allocated memory, see
 *  @ref dyna_shrink().
 *
 * @param array pointer to a dynamically allocated array.
 * @param index index at which to pop the item.
 * @return data at <index>, popped off the array.
 */
void*
dyna_pop(dyna_t* array, size_t index);

/**
 * @brief get the data at the index specified from a dynamically
 *  allocated array.
 *
 * @param array the dynamically allocated array to get information from.
 * @param index the index in the array that we are to retrieve data from.
 * @return 0x0 if the index is out of bounds or the data at a specified index in the array.
 */
void*
dyna_get(dyna_t* array, size_t index);

/**
 * @brief shrinks the array to the length via realloc.
 *
 * @param array the dynamically allocated array to be shrinked.
 */
void
dyna_shrink(dyna_t* array);
#endif //DYNA_H
