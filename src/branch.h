/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef BRANCH_H
#define BRANCH_H

/*! @uses sha1_t, sha1, sha256_t, sha256. */
#include "hash.h"

/*! @uses dyna_t, dyna_push, _foreach... */
#include "dyna.h"

/**
 * a data structure representing a branch within the version control system. a branch is a
 *  specific partition of the repository containing specified changes by the user, which can be
 *  back traced to the very first commit. it includes, a name, a path, a hash, a pointer to the
 *  head (current commit), and an array of the commits itself.
 */
typedef struct {
    char* name; /* branch name. */
    char* path; /* path to the branch directory. */
    sha1_t hash; /* hash of the branch. (pad 4 bytes) */
    size_t head; /* index to the head commit. */
    dyna_t* commits; /* array of commits hashes for this branch. */
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
 * @brief create a folder to hold all the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void
write_branch(const branch_t* branch);

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
read_branch(const char* name);
#endif /* BRANCH_H */