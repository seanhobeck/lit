/**
 * @author Sean Hobeck
 * @date 2025-11-12
 *
 * @file ops.h
 *    the operations module, it is responsible for handling
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
forward_commit_op(const commit_t* commit);

/**
 * @brief apply the commit backwards (inverse) to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
reverse_commit_op(const commit_t* commit);

/**
 * @brief rollback to an older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
rollback_op(branch_t* branch, const commit_t* commit);

/**
 * @brief checkout to a newer commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
checkout_op(branch_t* branch, const commit_t* commit);
#endif //OPS_H