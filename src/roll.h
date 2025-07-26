/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file roll.h
 *    the rollback module in the version control system, it is responsible for being
 *    able to checkout older versions of files in commits.
 */
#ifndef ROLL_H
#define ROLL_H

/*! @uses commit_t */
#include <commit.h>

/*! @uses branch_t */
#include <branch.h>

/**
 * @brief given a older diff, rollback to it (ignore anything -, keep ' ' and +)
 *
 * @param diff the diff provided (from a older commit).
 * @param n the number of lines in the rolled-back diff.
 * @return a allocated number of lines strdup'd.
 */
char ** rollback_to_diff(const diff_t* diff, int* n);

/**
 * @brief rollback / checkout a older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void rollback(branch_t* branch, const commit_t* commit);
#endif //ROLL_H