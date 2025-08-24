/**
 * @author Sean Hobeck
 * @date 2025-08-23
 *
 * @file branch.c
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#include "branch.h"

/*! @uses mkdir, getcwd. */
#include <sys/stat.h>

/**
 * @brief create a new branch with the given name.
 *
 * @param name the name of the branch to be created.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
create_branch(const char* name) {
    // assert on the name.
    assert(name != 0x0);

    // create a new branch structure.
    branch_t* branch = calloc(1ul, sizeof *branch);

    // copy the name into the branch structure.
    branch->name = calloc(1, strlen(name) + 1);
    strncpy(branch->name, name, strlen(name) + 1);

    // create the branch path based on the cwd.
    branch->path = calloc(1, 256);
    sprintf(branch->path, ".lit/refs/heads/%s", branch->name);

    // calculate the sha1 based on the name, and randomized pointer addresses in memory to the branch
    //  and to the branches fields (path and name) respectively.
    unsigned char* random = calloc(1, 256);
    snprintf((char*) random, 256, "%p%s%s", &branch, branch->name, branch->path);
    sha1(random, 256, branch->hash);
    free(random);
    return branch;
};

/**
 * @brief create a folder to hold all of the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void
write_branch(const branch_t* branch) {
    // assert on the branch ptr.
    assert(branch != 0x0);

    // create branch file.
    FILE* f = fopen(branch->path, "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open branch file for writing.\n");
        exit(EXIT_FAILURE); // exit on failure.
    }

    // write the branch name and hash to the file.
    fprintf(f, "name:%s\nsha1:%s\nidx:%lu\ncount:%lu\n", \
        branch->name, strsha1(branch->hash), branch->idx, branch->count);

    // for each commit in this branch, write out the respective sha1 hash.
    for (size_t i = 0; i < branch->count; i++)
        fprintf(f, "%s\n", strsha1(branch->commits[i]->hash));
    fclose(f);
};

/**
 * @brief add a commit to the branch history.
 *
 * @param commit the commit_t structure to be added to the branch history.
 * @param branch the history_t structure to which the commit will be added.
 */
void
add_commit_branch(commit_t* commit, branch_t* branch) {
    // assert on the commit and the branch.
    assert(branch != 0x0);
    assert(commit != 0x0);

    // if the count is greater than the capacity, realloc.
    if (branch->count >= branch->capacity) {
        branch->capacity = branch->capacity ? branch->capacity * 2 : 1; // increment the commit count.
        commit_t** commits = realloc(branch->commits, sizeof(commit_t*) * branch->capacity);
        if (!commits) {
            fprintf(stderr,"realloc failed; could not reallocate memory for branch.\n");
            exit(EXIT_FAILURE); // exit on failure.
        }
        branch->commits = commits;
    }
    // copy the commit into the history.
    branch->commits[branch->count++] = commit;
};

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t*
read_branch(const char* name) {
    // assert on the name.
    assert(name != 0x0);

    // create a temporary branch structure.
    branch_t* branch = calloc(1, sizeof *branch);

    // create the branch path based on the cwd.
    branch->path = calloc(1, 256);
    sprintf(branch->path, ".lit/refs/heads/%s", name);

    // open the branch file for reading.
    FILE* f = fopen(branch->path, "r");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open branch file for reading.\n");
        exit(EXIT_FAILURE); // exit on failure.
    }

    // read the branch information from the file.
    size_t count = 0;
    branch->name = calloc(1, 129);
    char *branch_hash = calloc(1, 41);
    int scanned = fscanf(f, "name:%128[^\n]\nsha1:%40[^\n]\nidx:%lu\ncount:%lu\n", \
        branch->name, branch_hash, &branch->idx, &count);
    if (scanned != 4) {
        fprintf(stderr,"fscanf failed; could not read branch header.\n");
        fclose(f);
        exit(EXIT_FAILURE); // exit on failure.
    }

    // conversion to hashes.
    unsigned char* _hash = strtoha(branch_hash, 20);
    memcpy(branch->hash, _hash, 20);
    free(_hash);
    free(branch_hash);

    // start reading the file for the count of commits, then
    // use the first byte (2 chars) as the folder path in .lit/objects/commits/xx
    // and then the rest (name+2) as the file name.
    for (size_t i = 0; i < count; i++) {
        char* hash = calloc(1, 41);
        fscanf(f, "%40[^\n]\n", hash);

        // construct the file location from the hash.
        char path[256];
        snprintf(path, 256, ".lit/objects/commits/%.2s/%38s", hash, hash + 2);
        commit_t* commit = read_commit(path);
        add_commit_branch(commit, branch);
        free(hash);
    }
    return branch;
};