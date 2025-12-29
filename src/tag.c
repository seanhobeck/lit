/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#include "tag.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses exit, calloc. */
#include <stdlib.h>

/*! @uses strdup. */
#include "utl.h"

/*! @uses sha1_t, sha1. */
#include "hash.h"

/*! @uses vector_t, vector_collect, vector_free. */
#include "inw.h"

/*! @uses log, E_LOGGER_LEVEL_ERROR, E_LOGGER_LEVEL_INFO. */
#include "log.h"

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
    const char* name) {
    /* assert on all the pointers. */
    assert(branch != 0x0);
    assert(commit != 0x0);
    assert(name != 0x0);

    /* create our tag structure. */
    tag_t* tag = calloc(1, sizeof *tag);
    if (!tag) {
        log(E_LOGGER_LEVEL_ERROR, "calloc failed; could not allocate memory for tag.\n");
        exit(EXIT_FAILURE);
    }

    /* copy over all the information. */
    tag->name = strdup(name);
    memcpy(tag->commit_hash, commit->hash, 20u);
    memcpy(tag->branch_hash, branch->hash, 20u);
    return tag;
};

/**
 * @brief write a tag to '.lit/refs/tags/'.
 *
 * @param tag the tag to be written.
 */
void
write_tag(const tag_t* tag) {
    /* assert on the tag ptr. */
    assert(tag != 0x0);

    /* open the file given the path and the tag. */
    char path[256];
    snprintf(path, 256, ".lit/refs/tags/%s", tag->name);
    FILE* f = fopen(path, "w");
    if (!f) {
        log(E_LOGGER_LEVEL_ERROR, "fopen failed; could not open file for tag writing.\n");
        exit(EXIT_FAILURE);
    }

    /* then we write some data and close. */
    fprintf(f, "msg:%s\ncommit:%s\nbranch:%s\n", \
        tag->name, strsha1(tag->commit_hash), strsha1(tag->branch_hash));
    fclose(f);
};

/**
 * @brief read tags from the folder '.lit/refs/tags/'
 *
 * @return a dynamic array of allocated tag structures.
 */
dyna_t*
read_tags() {
    /* collect all tags in the './lit/refs/tags/' folder. */
    dyna_t* array = inw_walk(".lit/refs/tags", E_INW_TYPE_NO_RECURSE);

    /* create our tag list. */
    dyna_t* new_array = dyna_create(sizeof(tag_t*));

    // iterate through each inode.
    _foreach(array, inode_t*, node, i)
        /* ignore everything that is not a file. */
        if (node->type != E_INODE_TYPE_FILE)
            continue;

        /* allocate and open the file. */
        tag_t* tag = calloc(1, sizeof *tag);
        if (!tag) {
            fprintf(stderr, "calloc failed; could not allocate memory for tag.\n");
            exit(EXIT_FAILURE);
        }
        FILE* f = fopen(node->path, "r");
        if (!f) {
            fprintf(stderr, "fopen failed; could not open file for tag reading.\n");
            exit(EXIT_FAILURE);
        }

        /* read the file information. */
        tag->name = calloc(1, 1025);
        char* commit_hash = calloc(1, 41), *branch_hash = calloc(1, 41);
        int scanned = fscanf(f, "msg:%1024[^\n]\ncommit:%40[^\n]\nbranch:%40[^\n]\n", \
            tag->name, commit_hash, branch_hash);
        if (scanned != 3) {
            fprintf(stderr, "fscanf failed; could not read tag file \'%s\'\n", node->path);
            exit(EXIT_FAILURE);
        }

        /* convert the hashes. */
        unsigned char *_commit_hash = strtoha(commit_hash, 20), \
            *_branch_hash = strtoha(branch_hash, 20);
        free(commit_hash);
        free(branch_hash);

        /* copy the converted hashes over. */
        memcpy(tag->commit_hash, _commit_hash, 20);
        free(_commit_hash);
        memcpy(tag->branch_hash, _branch_hash, 20);
        free(_branch_hash);

        /* free and append. */
        fclose(f);
        dyna_push(new_array, tag);
    _endforeach;

    /* free and return. */
    dyna_free(array);
    return new_array;
};

/**
 * @brief filter tags based on the branch to a new array.
 *
 * @param branch_hash hash to filter for.
 * @param array the old dynamic array of tags to be filtered through.
 * @return a dynamic array of the tags within the repository.
 */
dyna_t*
filter_tags(const sha1_t branch_hash, const dyna_t* array) {
    /* assert on the tag array. */
    assert(array != 0x0);

    /* allocate our new array. */
    dyna_t* new_array = dyna_create(sizeof(tag_t*));

    /* iterate through the old array and run memcmp. */
    _foreach(array, tag_t*, tag, i)
        if (!memcmp(tag->branch_hash, branch_hash, 20))
            dyna_push(new_array, tag);
    _endforeach;
    return new_array;
};