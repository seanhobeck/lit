/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef REBASE_H
#define REBASE_H

/*! @uses repository_t, find_common_ancestor, find_index_commit. */
#include "repo.h"

/**
 * enum to describe the possible results of calling the rebase operation on two branches. these
 * are only used as a return for the function 'branch_rebase'.
 */
typedef enum {
    E_REBASE_RESULT_SUCCESS = 0x0,
    E_REBASE_RESULT_CONFLICT = 0x1,
    E_REBASE_RESULT_NO_CHANGES = 0x2
} e_rebase_result_t;

/**
 * @brief rebase the current/active branch onto the source branch.
 *
 * @param repository the repository read from the cwd.
 * @param destination_branch_name the destination branch to rebase onto.
 * @param source_branch_name the source branch to rebase from.
 * @return rebase result status enum.
 */
e_rebase_result_t
branch_rebase(const repository_t* repository, const char* destination_branch_name, \
    const char* source_branch_name);
#endif /* REBASE_H */