/**
 * @author Sean Hobeck
 * @date 2025-11-13
 *
 * @file cache.c
 *    the cache module, responsible for clearing any possible object
 *    caches that are not related to any commits.
 */
#include "cache.h"

/*! @uses vector_t*, vector_collect */
#include "inw.h"

/*! @uses bool, true, false */
#include <stdbool.h>

/**
 * @brief scan the .lit/objects folder for unrelated objects to any current branches,
 *  if any are found, remove them.
 *
 * @param repository the repository read in the cwd.
 * @return the result enum of scanning the .lit/objects folder for unrelated objects.
 */
e_cache_result_t
scan_object_cache(const repository_t* repository) {
    // default return code.
    e_cache_result_t result = E_CACHE_RESULT_NO_CACHE;
    if (!repository)
        return result;

    // scan the .lit/objects directory recursively to find all the objects.
    // then we look through each one of the inodes, if it is not found in there, we
    // then remove it as it isn't needed (cache).
    dyna_t* objects = inw_walk(".lit/objects", E_INW_TYPE_RECURSE);
    if (!objects) {
        fprintf(stderr, "failed to collect objects from the .lit/objects directory.\n");
        exit(EXIT_FAILURE);
    }

    // the count of how many objects have been removed.
    size_t count = 0;

    // iterate...
    for (size_t i = 0; i < objects->length; i++) {
        inode_t* node = dyna_get(objects, i);

        // if this is not a file, ignore it.
        if (node->type != E_INODE_TYPE_FILE)
            continue;

        // iterate through each change in each branch.
        bool is_referenced = false;
        for (size_t j = 0; j < repository->count; j++) {
            branch_t* branch = repository->branches[j];
            for (size_t k = 0; k < branch->count; k++) {
                commit_t* commit = branch->commits[k];

                // if the paths are equal.
                if (!strcmp(commit->path, node->path)) {
                    is_referenced = true;
                    break;
                }

                // iterate through diffs as well.
                for (size_t l = 0; l < commit->count; l++) {
                    diff_t* diff = commit->changes[l];

                    // construct the file location from the hash.
                    char* dpath = calloc(1, 256), *dhash = calloc(1, 128);
                    snprintf(dhash, 128, "%04u", diff->crc);
                    snprintf(dpath, 256, ".lit/objects/diffs/%.2s/%s", dhash, dhash + 2);

                    // if the paths are equal.
                    if (!strcmp(node->path, dpath)) {
                        is_referenced = true;
                        free(dpath);
                        free(dhash);
                        break;
                    }
                    free(dpath);
                    free(dhash);
                }
            }
        }

        // if the object is not referenced, remove it.
        if (!is_referenced) {
            // set the result and get the parent directory as well.
            result = E_CACHE_RESULT_SUCCESS;
            char* parent_path = rpwd(node->path);
            dyna_t* parent = inw_walk(parent_path, E_INW_TYPE_NO_RECURSE);

            // if this is the only object that needs to be removed in this folder, then remove the
            //  parent folder as well.
            remove(node->path);
            if (parent->length == 1)
                remove(parent_path);
            dyna_free(parent);
            count++;
        }
    }

    // print results.
    if (result == E_CACHE_RESULT_SUCCESS) 
        printf("cache cleaned successfully, removed %lu unreferenced objects.\n", count);

    // free and return the result.
    dyna_free(objects);
    return result;
};