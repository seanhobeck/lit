/**
 * @author Sean Hobeck
 * @date 2025-11-13
 *
 * @file tag.c
 *    the tag module, responsible for tagging any important rebases,
 *    merges and commits that the user deems to be important.
 */
#include "tag.h"

/*! @uses sha1_t, sha1 */
#include "hash.h"

/*! @uses vector_t, vector_collect, vector_free */
#include "inw.h"

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
    // assert on all of the pointers.
    assert(branch != 0x0);
    assert(commit != 0x0);
    assert(name != 0x0);

    // create our tag structure.
    tag_t* tag = calloc(1, sizeof *tag);
    if (!tag) {
        fprintf(stderr, "calloc failed; could not allocate memory for tag.\n");
        exit(EXIT_FAILURE);
    }

    // copy over all the information.
    tag->name = strdup(name);
    memcpy(tag->commit_hash, commit->hash, 20);
    memcpy(tag->branch_hash, branch->hash, 20);
    return tag;
};

/**
 * @brief write a tag to '.lit/refs/tags/'.
 *
 * @param tag the tag to be written.
 */
void
write_tag(const tag_t* tag) {
    // assert on the tag ptr.
    assert(tag != 0x0);

    // open the file given the path and the tag.
    char path[256];
    snprintf(path, 256, ".lit/refs/tags/%s", tag->name);
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "fopen failed; could not open file for tag writing.\n");
        exit(EXIT_FAILURE);
    }

    // then we write some data and close.
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
    // vector collect all tags in the './lit/refs/tags/' folder.
    dyna_t* array = inw_walk(".lit/refs/tags", E_INW_TYPE_NO_RECURSE);

    // create our tag list.
    dyna_t* new_array = dyna_create(sizeof(tag_t*));

    // iterate through each inode_t.
    for (size_t i = 0; i < array->length; i++) {
        inode_t* node = dyna_get(array, i);

        // ignore everything that is not a file.
        if (node->type != E_INODE_TYPE_FILE)
            continue;

        // allocate and open the file.
        tag_t* tag = calloc(1, sizeof *tag);
        FILE* f = fopen(node->path, "r");
        if (!tag) {
            fprintf(stderr, "calloc failed; could not allocate memory for tag.\n");
            exit(EXIT_FAILURE);
        }
        if (!f) {
            fprintf(stderr, "fopen failed; could not open file for tag reading.\n");
            exit(EXIT_FAILURE);
        }

        // read the file information using fscanf.
        tag->name = calloc(1, 1025);
        char* commit_hash = calloc(1, 41), *branch_hash = calloc(1, 41);
        int scanned = fscanf(f, "msg:%1024[^\n]\ncommit:%40[^\n]\nbranch:%40[^\n]\n", \
            tag->name, commit_hash, branch_hash);
        if (scanned != 3) {
            fprintf(stderr, "fscanf failed; could not read tag file \'%s\'\n", node->path);
            exit(EXIT_FAILURE);
        }

        // convert the hashes.
        unsigned char *_commit_hash = strtoha(commit_hash, 20), \
            *_branch_hash = strtoha(branch_hash, 20);
        memcpy(tag->commit_hash, _commit_hash, 20);
        memcpy(tag->branch_hash, _branch_hash, 20);

        // free and append
        free(commit_hash);
        free(branch_hash);
        free(_commit_hash);
        free(_branch_hash);
        fclose(f);
        dyna_push(new_array, tag);
    }
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
filter_tags(const sha1_t branch_hash, \
    const dyna_t* array) {
    // assert on the tag array.
    assert(array != 0x0);

    // allocate our new array.
    dyna_t* new_array = dyna_create(sizeof(tag_t*));

    // iterate through the old array and run memcmp.
    for (size_t i = 0; i < array->length; i++) {
        tag_t* tag = dyna_get(array, i);
        if (!memcmp(tag->branch_hash, branch_hash, 20))
            dyna_push(new_array, tag);
    }
    return new_array;
};