/**
 *	@author Sean Hobeck
 *	@date 2025-12-28
 */
#ifndef DYNA_H
#define DYNA_H

/*! @uses size_t. */
#include <stddef.h>

/**
 * a data structure for a dynamically allocated array, it can hold only a set item size, as well
 *  as anything less. although it can hold items less than it, meaning if you have a struct a and b
 *  where sizeof(a) >= sizeof(b), if the item size = sizeof(a), then you can technically hold
 *  struct b within the list, it is ideal if you stick with a constant item size, and create a new
 *  dynamically allocated list (dyna) with a new size, and copy all data over (or refer to
 *  dyna_upgradef).
 */
typedef struct {
    void** data; /* array of data. */
    /* length (count) and capacity of the dynamic array. */
    size_t length, capacity, isize; /* element size; sizeof the data in the array. */
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

/**
 * @brief upgrade the item size of a dynamically allocated array and then also
 *  free the provided array.
 *
 * @param old_array the dynamically allocated array to be upgraded (then freed).
 * @param new_size the new size of a dynamically allocated array to be upgraded.
 * @return a new dynamically allocated array containing a larger item size.
 */
dyna_t*
dyna_upgradef(dyna_t* old_array, size_t new_size);

/* a get operation. */
#define _get(array, type, index) ((type) dyna_get(array, index))

/* starting an iteration. */
#define _foreach_it(array, type, var, iter) \
    for (size_t iter = 0; iter < (array)->length; iter++) { \
        type var = _get(array, type, iter);

/* starting an iteration. */
#define _foreach(array, type, var) \
    for (size_t i = 0; i < (array)->length; i++) { \
        type var = _get(array, type, i);

/* starting an iteration, backwards. */
#define _inv_foreach(array, type, var) \
    for (size_t i = (array)->length; i != 0; i--) { \
        type var = _get(array, type, i - 1);

/* starting an iteration, backwards. */
#define _inv_foreach_it(array, type, var, iter) \
    for (size_t iter = (array)->length; iter != 0; iter--) { \
        type var = _get(array, type, iter - 1);

/* ending an iteration. */
#define _endforeach }

/* ending an iteration. */
#define _endforeach }
#endif /* DYNA_H */
