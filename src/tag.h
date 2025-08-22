/**
 * @author Sean Hobeck
 * @date 2025-08-15
 *
 * @file tag.h
 *    the tag module, responsible for tagging any important rebases,
 *    merges and commits that the user deems to be important.
 */
#ifndef TAG_H
#define TAG_H

/*! @uses branch_t */
#include "branch.h"

/*! @uses commit_t */
#include "commit.h"

/*! @uses sha1_t */
#include "hash.h"

/// @note a data structure to represent the tagging of a commit.
typedef struct {
    sha1_t commit_hash, branch_hash;
    char* name;
} tag_t;

/**
 * @brief create a tag for a commit with a message.
 *
 * @param branch the parent branch of the commit.
 * @param commit the commit to be tagged.
 * @param name the name to be added to the tag.
 * @return the tag structure we generate for a commit.
 */
tag_t*
create_tag(const branch_t* branch, const commit_t* commit, \
    const char* name);

/**
 * @brief write a tag to '.lit/refs/tags/'.
 *
 * @param tag the tag to be written.
 */
void
write_tag(const tag_t* tag);

/// @note a data structure for a list of tags.
typedef struct {
    size_t count, capacity;
    tag_t** tags;
} tag_list_t;

/**
 * @brief read tags from the folder '.lit/refs/tags/'
 *
 * @return a list of allocated tag structures as well as
 *  a count and capacity.
 */
tag_list_t*
read_tags();

/**
 * @brief filter tags based on the branch to a new list.
 *
 * @param branch_hash the branch hash to filter for.
 * @param tags the old list of tags to be filtered through.
 * @return a structure containing a dynamic list of allocated tags.
 */
tag_list_t*
filter_tags(const sha1_t branch_hash, \
    const tag_list_t* tags);
#endif TAG_H //TAG_H