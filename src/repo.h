/**
 * @author Sean Hobeck
 * @date 2025-07-22
 *
 * @file repo.h
 *    the repository module in the version control system, it is responsible for handling branches,
 *    and their locations, as well as the overall state of the repository.
 */
#ifndef REPO_H
#define REPO_H

/*! @uses branch_t. */
#include <branch.h>

/// @note a data structure to hold a repository, containing a main branch and a list of branches
///     if chosen to be used.
typedef struct {
    branch_t main;
    unsigned long n_branches; // number of branches in the repository.
    branch_t* branches; // array of branches in the repository.
} repository_t;

/**
 * @brief create a new repository with the given main branch name.
 *
 * @return a 0 if successful in creating the directory, or -1 if failed.
 */
int repository_create();

/**
 * @brief write the repository to disk in our '.lit' directory.
 */
void repository_write(const repository_t* repo);

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t repository_read();
#endif //REPO_H