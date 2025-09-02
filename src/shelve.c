/**
 * @author Sean Hobeck
 * @date 2025-08-26
 *
 * @file shelve.c
 *    the shelving module, responsible for being able to shelve
 *    changes between branches without committing them to the history;
 *    perforce-style shelving (per branch shelved changes that can be applied
 *    to that branch only).
 */
#include "shelve.h"

/*! @uses mkdir */
#include <sys/stat.h>

/**
 * @brief write changes to a shelved file on a branch.
 *
 * @param branch_name the name of the branch to shelve changes on.
 * @param diff the diff_t structure containing the changes to shelve.
 */
void
shelve_changes(const char* branch_name, const diff_t* diff) {
    // assert on the parameters.
    assert(branch_name != 0x0);
    assert(diff != 0x0);

    // ensure the shelved directory exists.
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch_name);
    if (fexistpd(path) == -1) {
        fprintf(stderr, "\'.lit/objects/shelved\' does not exist; possible branch corruption.\n");
        exit(EXIT_FAILURE);
    }
    mkdir(path, MKDIR_MOWNER);

    // create the file path.
    char shelved_path[512];
    snprintf(shelved_path, 512, "%s/%u.diff", path, diff->crc);

    // write the diff and log.
    write_diff(diff, shelved_path);
};

/**
 * @brief collect shelved changes for a branch.
 *
 * @param branch_name the branch name to collect shelved changes for.
 * @return a vector_t* containing the vinode_t* of the shelved changes.
 */
vector_t*
shelve_collect(const char* branch_name) {
    // assert on the branch name.
    assert(branch_name != 0x0);

    // create the path and then collect the files.
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch_name);
    return vector_collect(path, E_PVC_TYPE_NO_RECURSE);
};