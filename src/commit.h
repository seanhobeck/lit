/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef COMMIT_H
#define COMMIT_H

/*! @uses sha1_t, sha1, strsha1. */
#include "hash.h"

/*! @uses FILE*. */
#include <stdio.h>

/*! @uses time_t */
#include <time.h>

/*! @uses dyna_t, dyna_push, _foreach, ...*/
#include "dyna.h"

/**
 * a data structure containing information on a commit, think a list of changes made on disk that
 *  can be tracked to a specific unique identifier; all changes being tracked in one singular
 *  motion. these commits contain a timestamp as well as a path, message, and hash. the entire
 *  basis of a version control system like this is based on these lists of changes/ commits.
 */
typedef struct {
    dyna_t* changes; /* array of diffs in the commit. */
    char* timestamp; /* commit timestamp. */
    time_t rawtime; /* raw time representation as an integer. */
    char* path; /* path to the commit directory. */
    char* message; /* commit message. */
    sha1_t hash; /* sha1 hash of the commit. */
} commit_t;

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @param branch_name the name of the parent branch that this commit is being placed under.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
create_commit(const char* message, const char* branch_name);

/**
 * @brief write the commit to a file in our '.lit' directory under our current branch.
 *
 * @param commit the commit_t structure to be written to a file.
 */
void
write_commit(const commit_t* commit);

/**
 * @brief write the commit pointer to a generic open file stream.
 *
 * @param stream the stream from fopen() on some file descriptor.
 * @param commit the commit to be written to the stream.
 */
void
write_commit_to_stream(FILE* stream, const commit_t* commit);

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
read_commit(const char* path);

/**
 * @brief read a commit from a generic open file stream.
 *
 * @param stream the stream from which to read from.
 * @return a pointer to an allocated commit with all the data.
 */
commit_t*
read_commit_from_stream(FILE* stream);
#endif /* COMMIT_H */