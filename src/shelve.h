/**
 * @author Sean Hobeck
 * @date 2025-08-26
 *
 * @file shelve.h
 *    the shelving module, responsible for being able to shelve
 *    changes between branches without committing them to the history;
 *    perforce-style shelving (per branch shelved changes that can be applied
 *    to that branch only).
 */
#ifndef SHELVE_H
#define SHELVE_H

/*! @uses diff_t, size_t */
#include "diff.h"

/*! @uses vector_t */
#include "pvc.h"

/**
 * @brief write changes to a shelved file on a branch.
 *
 * @param branch_name the name of the branch to shelve changes on.
 * @param diff the diff_t structure containing the changes to shelve.
 */
void
shelve_changes(const char* branch_name, const diff_t* diff);

/**
 * @brief collect shelved changes for a branch.
 *
 * @param branch_name the branch name to collect shelved changes for.
 * @return a vector_t* containing the vinode_t* of the shelved changes.
 */
vector_t*
shelve_collect(const char* branch_name);
#endif //SHELVE_H