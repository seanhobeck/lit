/**
 * @author Sean Hobeck
 * @date 2025-12-27
 */
#ifndef CACHE_H
#define CACHE_H

/*! @uses repository_t. */
#include "../core/repo.h"

/** enum for all possible cache results; some of the explanations provided below. */
typedef enum {
    E_CACHE_RESULT_SUCCESS = 0x0, /* if we found some caches that could be removed. */
    E_CACHE_RESULT_NO_CACHE = 0x1, /* if we did not find any caches to remove. */
    E_CACHE_RESULT_ERROR = 0x2, /* if we encountered an error while scanning caches. */
} e_cache_result_t;

/**
 * @brief scan the .lit/objects folder for unrelated objects to any current branches,
 *  if any are found, remove them.
 *
 * @param repository the repository read in the cwd.
 * @return the result enum of scanning the .lit/objects folder for unrelated objects.
 */
e_cache_result_t
scan_object_cache(const repository_t* repository);
#endif /* CACHE_H */