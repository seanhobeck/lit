/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef TAG_H
#define TAG_H

/*! @uses branch_t. */
#include "branch.h"

/*! @uses commit_t. */
#include "commit.h"

/*! @uses dyna_t. */
#include "dyna.h"

/*! @uses sha1_t. */
#include "hash.h"

/**
 * a data structure representing a tag used within the version control system; a marker on a certain
 *  commit for the users to use within the command line. for example, if there is a specific
 *  commit that a user wishes to pin or not have to look for the hash (which can be very tedeous),
 *  it can be tagged within the repository.
 */
typedef struct {
    /* parent branch, commit hash, and name. */
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

/**
 * @brief read tags from the folder '.lit/refs/tags/'
 *
 * @return a dynamic array of allocated tag structures.
 */
dyna_t*
read_tags();

/**
 * @brief filter tags based on the branch to a new array.
 *
 * @param branch_hash hash to filter for.
 * @param array the old dynamic array of tags to be filtered through.
 * @return a dynamic array of the tags within the repository.
 */
dyna_t*
filter_tags(const sha1_t branch_hash, dyna_t* array);
#endif /* TAG_H */