/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file branch.h
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#ifndef BRANCH_H
#define BRANCH_H

/*! @uses commit_t. */
#include <commit.h>

/*! @uses sha256_t. */
#include <hash.h>

/// @note a data structure to hold a commit history for a certain branch.
typedef struct {
    unsigned long n_commits; // number of commits in the history.
    commit_t** commits; // array of commits in the history.
} history_t;

/// @note a data structure to hold a branch, containing commits and our current commit.
typedef struct {
    char name[128u]; // branch name.
    char path[256u]; // path to the branch directory.
    sha256_t hash; // sha256 hash of the branch.
    history_t history; // history of commits for the branch.
    unsigned long current_commit; // index of the current commit in the branch.
    sha1_t current_commit_hash; // sha1 hash of the current commit.
} branch_t;

/**
 * @brief create a new branch with the given name.
 *
 * @param name the name of the branch to be created.
 * @return a branch_t structure containing the branch information.
 */
branch_t branch_create(const char* name);

/**
 * @brief create a folder to hold all of the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void branch_write(const branch_t* branch);

/**
 * @brief add a commit to the branch history.
 *
 * @param commit the commit_t structure to be added to the branch history.
 * @param history the history_t structure to which the commit will be added.
 */
void branch_add_commit(commit_t* commit, history_t* history);

/**
 * @brief read the commit history from a branch path.
 *
 * @param path the path to the branch history file.
 * @return a history_t structure containing the commit history.
 */
history_t branch_read_history(const char* path);

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t branch_read(const char* name);
#endif //BRANCH_H