/**
 * @author Sean Hobeck
 * @date 2026-01-06
 */
#include "../shelve.h"

/*! @uses mkdir. */
#include <sys/stat.h>

/*! @uses assert. */
#include <assert.h>

/*! @uses snprintf. */
#include <stdio.h>

/*! @uses exit. */
#include <stdlib.h>

/*! @uses fexistpd. */
#include "../utl.h"

/*! @uses log, E_LOGGER_LEVEL_ERROR, E_LOGGER_LEVEL_INFO. */
#include "log.h"

/**
 * @brief write changes to a shelved file on a branch.
 *
 * @param branch_name the name of the branch to shelve changes on.
 * @param diff the diff_t structure containing the changes to shelve.
 */
void
write_to_shelved(const char* branch_name, const diff_t* diff) {
    /* assert on the parameters. */
    assert(branch_name != 0x0);
    assert(diff != 0x0);

    /* ensure the shelved directory exists. */
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch_name);
    if (fexistpd(path) == -1) {
        llog(E_LOGGER_LEVEL_ERROR, "'.lit/objects/shelved' does not exist; possible branch corruption.");
        exit(EXIT_FAILURE);
    }
    mkdir(path, MKDIR_MOWNER);

    /* create the file path. */
    char shelved_path[512];
    snprintf(shelved_path, 512, "%s/%u.diff", path, diff->crc);

    /* write the diff and log. */
    write_diff(diff, shelved_path);
    llog(E_LOGGER_LEVEL_INFO, "shelved changes on branch \'%s\'.\n", branch_name);
}

/**
 * @brief collect shelved changes for a branch.
 *
 * @param branch_name the branch name to collect shelved changes for.
 * @return a dynamic array containing the pointer to each inode within the shelved changes.
 */
dyna_t*
collect_shelved(const char* branch_name) {
    /* assert on the branch name. */
    assert(branch_name != 0x0);

    /* create the path and then collect the files. */
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch_name);
    return inw_walk(path, E_INW_TYPE_NO_RECURSE);
}