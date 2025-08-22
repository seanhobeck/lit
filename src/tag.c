/**
 * @author Sean Hobeck
 * @date 2025-08-15
 *
 * @file tag.c
 *    the tag module, responsible for tagging any important rebases,
 *    merges and commits that the user deems to be important.
 */
#include "tag.h"

/*! @uses sha1_t, sha1 */
#include "hash.h"

/*! @uses vector_t, vector_collect, vector_free */
#include "pvc.h"

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
    // create our tag structure.
    tag_t* tag = calloc(1, sizeof *tag);
    if (!tag) {
        fprintf(stderr, "calloc failed; could not allocate memory for tag.\n");
        exit(EXIT_FAILURE);
    }

    // copy over all of the information.
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
 * @brief append a tag to a list of tags.
 *
 * @param list the list of tags to append the tag to.
 * @param tag the tag to be appended.
 */
void append_tag(tag_list_t* list, tag_t* tag) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 1;
        list->tags = realloc(list->tags, sizeof(tag_t*) * list->capacity);
        if (!list->tags) {
            fprintf(stderr, "realloc failed; could not allocate memory for tag list.\n");
            exit(EXIT_FAILURE);
        }
    }
    list->tags[list->count++] = tag;
};

/**
 * @brief read tags from the folder '.lit/refs/tags/'
 *
 * @return a list of allocated tag structures as well as
 *  a count and capacity.
 */
tag_list_t*
read_tags() {
    // vector collect all tags in the './lit/refs/tags/' folder.
    vector_t* vector = vector_collect(".lit/refs/tags", E_PVC_TYPE_NO_RECURSE);
    if (!vector) {
        fprintf(stderr, "vector_collect failed; could not collect tags.\n");
        exit(EXIT_FAILURE);
    }

    // create our tag list.
    tag_list_t* list = calloc(1, sizeof *list);
    if (!list) {
        fprintf(stderr, "calloc failed; could not allocate memory for tag list.\n");
        exit(EXIT_FAILURE);
    }

    // iterate through each vinode_t.
    for (size_t i = 0; i < vector->count; i++) {
        vinode_t* node = vector->nodes[i];

        // ignore everything that is not a file.
        if (node->type != E_PVC_INODE_TYPE_FILE)
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

        // conver the hashes.
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
        append_tag(list, tag);
    }
    vector_free(vector);
    return list;
};

/**
 * @brief filter tags based on the branch to a new list.
 *
 * @param branch_hash the branch hash to filter for.
 * @param tags the old list of tags to be filtered through.
 * @return a structure containing a dynamic list of allocated tags.
 */
tag_list_t*
filter_tags(const sha1_t branch_hash, \
    const tag_list_t* tags) {
    // allocate our new list.
    tag_list_t* new_list = calloc(1, sizeof *new_list);
    if (!new_list) {
        fprintf(stderr, "calloc failed; could not allocate memory for tag list.\n");
        exit(EXIT_FAILURE);
    }

    // iterate through the old list, and run memcmp.
    for (size_t i = 0; i < tags->count; i++) {
        tag_t* tag = tags->tags[i];
        if (!memcmp(tag->branch_hash, branch_hash, 20))
            append_tag(new_list, tag);
    }
    return new_list;
};