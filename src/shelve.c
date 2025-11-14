/**
 * @author Sean Hobeck
 * @date 2025-11-12
 *
 * @file shelve.c
 *    the shelving module, responsible for being able to shelve
 *    changes between branches without committing them to the history;
 *    perforce-style shelving (per branch shelved changes that can be applied
 *    to that branch only).
 */
#include "shelve.h"

/*! @uses mkdir. */
#include <sys/stat.h>

/**
 * @brief write changes to a shelved file on a branch.
 *
 * @param branch_name the name of the branch to shelve changes on.
 * @param diff the diff_t structure containing the changes to shelve.
 */
void
write_to_shelved(const char* branch_name, const diff_t* diff) {
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
    printf("shelved changes on branch \'%s\'.\n", branch_name);
};

/**
 * @brief collect shelved changes for a branch.
 *
 * @param branch_name the branch name to collect shelved changes for.
 * @return a dynamic array containing the pointer to each inode within the shelved changes.
 */
dyna_t*
collect_shelved(const char* branch_name) {
    // assert on the branch name.
    assert(branch_name != 0x0);

    // create the path and then collect the files.
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch_name);
    return inw_walk(path, E_INW_TYPE_NO_RECURSE);
};