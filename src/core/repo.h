/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#ifndef REPO_H
#define REPO_H

/*! @uses bool, true, false*/
#include <stdbool.h>

/*! @uses branch_t. */
#include "branch.h"

/*! @uses commit_t. */
#include "commit.h"

/**
 * a data structure ...
 */
typedef struct {
    bool readonly; /* whether the repository is in read-only mode. */
    size_t idx; /* head branch in the repository. */
    dyna_t* branches; /* array of branches in the repository. */
} repository_t;

/**
 * @brief create a new repository with the given main branch name.
 *
 * @return a 0 if successful in creating the directory, or -1 if .lit directory already exists.
 */
repository_t*
create_repository();

/**
 * @brief write the repository to disk in our '.lit' directory.
 *
 * @param repo the repository_t structure to be written to a file.
 */
void
write_repository(const repository_t* repo);

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t*
read_repository();

/**
 * @brief create a new branch from the current branches HEAD commit.
 *
 * @param repository the repository provided.
 * @param name the name of the new branch.
 * @param from_name the name of the branch to copy the head from.
 */
void
create_branch_repository(repository_t* repository, const char* name, const char* from_name);

/**
 * @brief delete a branch from the repository.
 *
 * @param repository the repository provided.
 * @param name the name of the branch.
 */
void
delete_branch_repository(repository_t* repository, const char* name);

/**
 * @brief get the branch from the repository with error checking.
 *
 * @param repository the repository read from the cwd.
 * @param branch_name the name of the branch to get.
 * @return a branch_t* if the branch was found, and EXIT_FAILURE if it wasn't.
 */
branch_t*
get_branch_repository(const repository_t* repository, const char* branch_name);

/**
 * @brief switch to the branch provided.
 *
 * @param repository the repository provided.
 * @param name the name of the new branch.
 */
void
switch_branch_repository(repository_t* repository, const char* name);

/**
 * @brief find a common commit ancestor using hashes and timestamps by going backwards.
 *
 * @param branch1 the first branch.
 * @param branch2 the second branch.
 * @return pointer to the common ancestor commit, or 0x0 if none found.
 */
commit_t*
find_common_ancestor(branch_t* branch1, branch_t* branch2);

/**
 * @brief find the index of a commit in a branch.
 *
 * @param branch the branch to be searched.
 * @param commit the commit to find.
 * @return the index of the commit or -1 if not found.
 */
long
find_index_commit(branch_t* branch, commit_t* commit);
#endif /* REPO_H */