/**
 * @author Sean Hobeck
 * @date 2026-01-09
 */
#include "branch.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses strlen, strncpy. */
#include <string.h>

/*! @uses strtoha. */
#include "utl.h"

/*! @uses commit_t. */
#include "commit.h"

/*! @uses llog, E_LOGGER_LEVEL_ERROR. */
#include "log.h"


/**
 * @brief create a new branch with the given name.
 *
 * @param name the name of the branch to be created.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
create_branch(const char* name) {
    /* assert on the name. */
    assert(name != 0x0);

    /* create a new branch structure. */
    branch_t* branch = calloc(1ul, sizeof *branch);

    /* copy the name into the branch structure. */
    branch->name = calloc(1, strlen(name) + 1);
    strncpy(branch->name, name, strlen(name) + 1);

    /* create the dynamic array. */
    branch->commits = dyna_create();

    /* create the branch path based on the cwd. */
    branch->path = calloc(1, PATH_MAX);
    sprintf(branch->path, ".lit/refs/heads/%s", branch->name);

    /* calculate the sha1 based on the name, and randomized pointer addresses in memory to the
     *  branch and to the branches fields (path and name) respectively. todo: fix this. */
    unsigned char* random = calloc(1, 256);
    snprintf((char*) random, 256, "%p%s%s", &branch, branch->name, branch->path);
    sha1(random, 256, branch->hash);
    free(random);
    return branch;
}

/**
 * @brief create a folder to hold all the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void
write_branch(const branch_t* branch) {
    /* assert on the branch ptr. */
    assert(branch != 0x0);

    /* create a file for the branch. */
    FILE* stream = fopen(branch->path, "w");
    if (!stream) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open branch file for writing.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* call to write everything. */
    write_branch_to_stream(stream, branch);
    fclose(stream);
}

/**
 * @brief write the branch pointer to a generic open file stream.
 *
 * @param stream the stream from fopen() on some file descriptor.
 * @param branch the branch to be written to the stream.
 */
void
write_branch_to_stream(FILE* stream, const branch_t* branch) {
    /* write the branch name and hash to the stream. */
    fprintf(stream, "name:%s\nsha1:%s\nidx:%zu\ncount:%zu\n", \
        branch->name, strsha1(branch->hash), branch->head, branch->commits->length);

    /* write out each respective sha1 hash for every commit. */
    _foreach(branch->commits, commit_t*, commit)
        fprintf(stream, "%s\n", strsha1(commit->hash));
    _endforeach;
}

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
read_branch(const char* name) {
    /* assert on the name. */
    assert(name != 0x0);

    /* create the branch path based on the cwd. */
    char* path = calloc(1, PATH_MAX);
    sprintf(path, ".lit/refs/heads/%s", name);

    /* open the branch file for reading. */
    FILE* stream = fopen(path, "r");
    if (!stream) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open branch file for reading.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* call function to do this for us. */
    branch_t* result = read_branch_from_stream(stream);
    result->path = strdup(path);
    free(path);
    return result;
}

/**
 * @brief read a branch from a generic open file stream.
 *
 * @param stream the stream from which to read from.
 * @return a pointer to an allocated branch with all the data.
 */
branch_t*
read_branch_from_stream(FILE* stream) {
    /* create a temporary branch structure. */
    branch_t* branch = calloc(1, sizeof *branch);

    /* create the dynamic array as well. */
    branch->commits = dyna_create();

    /* read the branch information from the file. */
    branch->name = calloc(1, NAME_MAX_CHARS + 1);
    char* branch_hash = calloc(1, SHA1_MAX_CHARS + 1);
    char* head_string = calloc(1, 64 + 1);
    char* count_string = calloc(1, 64 + 1); /* ^ plus null term. */
    int scanned = fscanf(stream,
        "name:%128[^\n]\n"
        "sha1:%41[^\n]\n"
        "idx:%64[0-9]\n"
        "count:%64[0-9]\n", \
        branch->name,
        branch_hash,
        head_string,
        count_string);
    if (scanned != 4) {
        llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read branch header.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* convert to integers using sstrtosz. */
    size_t count = sstrtosz(count_string);
    branch->head = sstrtosz(head_string);
    free(count_string);
    free(head_string);

    /* convert to hashes. */
    unsigned char* temp_hash = strtoha(branch_hash, SHA1_SIZE);
    memcpy(branch->hash, temp_hash, SHA1_SIZE);
    free(branch_hash);
    free(temp_hash);

    /* read all the hashes for the commits. */
    for (size_t i = 0; i < count; i++) {
        /* allocate and scan the hash. */
        char* hash = calloc(1, SHA1_MAX_CHARS + 1);
        if (fscanf(stream, "%40[^\n]\n", hash) != 1) {
            llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read commit hash for branch.\n");
            exit(EXIT_FAILURE);
        }

        /* construct the file location from the hash. */
        char* path = calloc(1, PATH_MAX);
        sprintf(path, ".lit/objects/commits/%.2s/%38s", hash, hash + 2u);
        commit_t* commit = read_commit(path);

        /* push to the dynamic array. */
        dyna_push(branch->commits, commit);
        free(hash);
        free(path);
    }

    /* return the branch we have read. */
    return branch;
}