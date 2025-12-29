/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#include "cache.h"

/*! @uses inode_t*, inw_walk. */
#include "inw.h"

/*! @uses bool, true, false. */
#include <stdbool.h>

/*! @uses fprintf, printf. */
#include <stdio.h>

/*! @uses exit, free, calloc. */
#include <stdlib.h>

/*! @uses snprintf. */
#include <string.h>

/*! @uses diff_t. */
#include "diff.h"

/*! @uses commit_t. */
#include "commit.h"

/*! @uses rpwd. */
#include "utl.h"

/**
 * @brief scan the .lit/objects folder for unrelated objects to any current branches,
 *  if any are found, remove them.
 *
 * @param repository the repository read in the cwd.
 * @return the result enum of scanning the .lit/objects folder for unrelated objects.
 */
e_cache_result_t
scan_object_cache(const repository_t* repository) {
    /* default return code. */
    e_cache_result_t result = E_CACHE_RESULT_NO_CACHE;
    if (!repository)
        return result;

    /* scan the .lit/objects directory recursively to find all the objects. then we look through
     * each one of the inodes, if it is not found in there, we then remove it as it isn't necessary
     * (cache). */
    dyna_t* objects = inw_walk(".lit/objects", E_INW_TYPE_RECURSE);
    if (!objects) {
        fprintf(stderr, "failed to collect objects from the .lit/objects directory.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* the count of how many objects have been removed. */
    size_t count = 0;

    /* iterate... */
    _foreach(objects, inode_t*, node, i)
        /* if this is not a file, ignore it. */
        if (node->type != E_INODE_TYPE_FILE)
            continue;

        /* iterate through each change in each branch; todo change this so that there is some
         * type of dirty bit to each of the branch files as they are modified, decreasing
         * complexity. */
        bool is_referenced = false;
        _foreach(repository->branches, const branch_t*, branch, j)
            _foreach(branch->commits, commit_t*, commit, k)
                /* if the paths are equal. */
                if (!strcmp(commit->path, node->path)) {
                    is_referenced = true;
                    break;
                }

                /* iterate through diffs as well. */
                _foreach(commit->changes, const diff_t*, change, l)
                    /* construct the file location from the hash. */
                    char* filepath = calloc(1, 256), *hash = calloc(1, 128);
                    snprintf(hash, 128, "%04u", change->crc);
                    snprintf(filepath, 256, ".lit/objects/diffs/%.2s/%s", hash, hash + 2);

                    /* if the paths are equal. */
                    bool flag = !strcmp(node->path, filepath);
                    free(filepath);
                    free(hash);
                    if (flag) {
                        is_referenced = true;
                        break;
                    }
                _endforeach;
            _endforeach;
        _endforeach;

        /* if the object is not referenced, remove it. */
        if (!is_referenced) {
            /* set the result and get the parent directory as well. */
            result = E_CACHE_RESULT_SUCCESS;
            char* parent_path = rpwd(node->path);
            dyna_t* parent = inw_walk(parent_path, E_INW_TYPE_NO_RECURSE);

            /* if this is the only object that needs to be removed in this folder, then remove the
             *  parent folder as well. */
            remove(node->path);
            if (parent->length == 1)
                remove(parent_path);

            /* free and increment. */
            dyna_free(parent);
            count++;
        }
    _endforeach;

    /* print results. */
    if (result == E_CACHE_RESULT_SUCCESS) 
        printf("cache cleaned successfully, removed %lu unreferenced objects.\n", count);

    /* free and return the result. */
    dyna_free(objects);
    return result;
};