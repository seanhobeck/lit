/**
 * @author Sean Hobeck
 * @date 2025-08-12
 *
 * @file ops.h
 *    the branch module in the version control system, it is responsible for handling
 *    commits and applying changes between branches.
 */
#ifndef OPS_H
#define OPS_H

/*! @uses commit_t */
#include "commit.h"

/*! @uses branch_t */
#include "branch.h"

/**
 * @brief apply the commit forward to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
apply_forward_commit(const commit_t* commit);

/**
 * @brief apply the commit backwards (inverse) to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
apply_inverse_commit(const commit_t* commit);

/**
 * @brief rollback / checkout a older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
rollback(branch_t* branch, const commit_t* commit);

/**
 * @brief checkout to a newer commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
checkout(branch_t* branch, const commit_t* commit);
#endif //OPS_H