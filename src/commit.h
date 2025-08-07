/**
 * @author Sean Hobeck
 * @date 2025-07-29
 *
 * @file commit.h
 *    the commit module in the version control system, it is responsible for handling
 *    diffs and commits, as well as writing them to disk.
 */
#ifndef COMMIT_H
#define COMMIT_H

/*! @uses diff_t. */
#include "diff.h"

/*! @uses sha1_t, sha1, strsha1. */
#include "hash.h"

/// @note a data structure to hold a commit, this will contain
///     a message, a sha1 hash, and a timestamp.
typedef struct {
    size_t count, capacity; // number of diffs in the commit.
    diff_t** changes; // array of diffs in the commit.
    char* timestamp; // commit timestamp.
    char* path; // path to the commit directory.
    char* message; // commit message.
    sha1_t hash; // sha1 hash of the commit.
} commit_t;

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @param branch_name the name of the parent branch that this commit is being placed under.
 * @return a commit_t structure containing the commit information.
 */
commit_t* commit_create(const char* message, char* branch_name);

/**
 * @brief add a diff to the commit, this will be used to store
 *  information about changes made to files or directories.
 *
 * @param commit the commit_t structure to which the diff will be added.
 * @param diff the diff_t structure containing the differences to be added.
 */
void commit_add_diff(commit_t* commit, diff_t* diff);

/**
 * @brief write the commit to a file in our '.lit' directory under our current branch.
 *
 * @param commit the commit_t structure to to be written to a file.
 */
void commit_write(const commit_t* commit);

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t* commit_read(const char* path);
#endif //COMMIT_H