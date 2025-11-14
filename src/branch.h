/**
 * @author Sean Hobeck
 * @date 2025-08-12
 *
 * @file branch.h
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#ifndef BRANCH_H
#define BRANCH_H

/*! @uses commit_t. */
#include "commit.h"

/*! @uses sha1_t, sha1, sha256_t, sha256. */
#include "hash.h"

/// @note a data structure to hold a branch, containing commits and our current commit.
typedef struct {
    char* name; // branch name.
    char* path; // path to the branch directory.
    sha1_t hash; // sha1 hash of the branch.
    size_t count, head, capacity; // number of commits in the branch.
    commit_t** commits; // array of commits hashes for this branch.
} branch_t;

/**
 * @brief create a new branch with the given name.
 *
 * @param name the name of the branch to be created.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
create_branch(const char* name);

/**
 * @brief create a folder to hold all of the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void
write_branch(const branch_t* branch);

/**
 * @brief add a commit to the branch history.
 *
 * @param commit the commit_t structure to be added to the branch history.
 * @param branch the history_t structure to which the commit will be added.
 */
void
add_commit_branch(commit_t* commit, branch_t* branch);

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
read_branch(const char* name);
#endif //BRANCH_H