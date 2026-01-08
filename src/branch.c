/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#include "branch.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses strlen, strncpy. */
#include <string.h>

/*! @uses snprintf. */
#include <stdio.h>

/*! @uses strtoha. */
#include "utl.h"

/*! @uses commit_t. */
#include "commit.h"

/*!~ @note this is a format for the main parts of data that are written at the start (header) of
 *  the file for a branch stored within the repository. */
#define BRANCH_HEADER_FORMAT "name:%128[^\n]\nsha1:%40[^\n]\nidx:%lu\ncount:%lu\n"


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
    branch->path = calloc(1, 256);
    sprintf(branch->path, ".lit/refs/heads/%s", branch->name);

    /* calculate the sha1 based on the name, and randomized pointer addresses in memory to the
     *  branch and to the branches fields (path and name) respectively. */
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
    FILE* f = fopen(branch->path, "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open branch file for writing.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* write the branch name and hash to the file. */
    fprintf(f, "name:%s\nsha1:%s\nidx:%lu\ncount:%lu\n", \
        branch->name, strsha1(branch->hash), branch->head, branch->commits->length);

    /* write out each respective sha1 hash for every commit. */
    _foreach(branch->commits, commit_t*, commit)
        fprintf(f, "%s\n", strsha1(commit->hash));
    _endforeach;
    fclose(f);
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

    /* create a temporary branch structure. */
    branch_t* branch = calloc(1, sizeof *branch);

    /* create the dynamic array as well. */
    branch->commits = dyna_create();

    /* create the branch path based on the cwd. */
    branch->path = calloc(1, 256);
    sprintf(branch->path, ".lit/refs/heads/%s", name);

    /* open the branch file for reading. */
    FILE* f = fopen(branch->path, "r");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open branch file for reading.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* read the branch information from the file. */
    size_t count = 0;
    branch->name = calloc(1, 129);
    char *branch_hash = calloc(1, 41);
    int scanned = fscanf(f, BRANCH_HEADER_FORMAT, \
        branch->name, branch_hash, &branch->head, &count);
    if (scanned != 4) {
        fprintf(stderr,"fscanf failed; could not read branch header.\n");
        fclose(f);
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* convert to hashes. */
    unsigned char* _hash = strtoha(branch_hash, 20);
    memcpy(branch->hash, _hash, 20);
    free(_hash);
    free(branch_hash);

    /* start reading the file for the count of commits, then use the first byte (2 chars) as the
     *  folder path in .lit/objects/commits/xx, and then the rest (name+2) as the file name. */
    for (size_t i = 0; i < count; i++) {
        /* allocate and scan the hash. */
        char* hash = calloc(1, 41);
        fscanf(f, "%40[^\n]\n", hash);

        /* construct the file location from the hash. */
        char path[256];
        snprintf(path, 256, ".lit/objects/commits/%.2s/%38s", hash, hash + 2);
        commit_t* commit = read_commit(path);

        /* push to the dynamic array. */
        dyna_push(branch->commits, commit);
        free(hash);
    }

    /* return the branch we have read. */
    return branch;
}