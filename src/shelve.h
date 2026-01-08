/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef SHELVE_H
#define SHELVE_H

/*! @uses diff_t, size_t. */
#include "diff.h"

/*! @uses dyna_t, inw_walk. */
#include "inw.h"

/**
 * @brief write changes to a shelved file on a branch.
 *
 * @param branch_name the name of the branch to shelve changes on.
 * @param diff the diff_t structure containing the changes to shelve.
 */
void
write_to_shelved(const char* branch_name, const diff_t* diff);

/**
 * @brief collect shelved changes for a branch.
 *
 * @param branch_name the branch name to collect shelved changes for.
 * @return a dynamic array containing the pointer to each inode within the shelved changes.
 */
dyna_t*
collect_shelved(const char* branch_name);
#endif /* SHELVE_H */