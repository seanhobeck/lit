/**
 * @author Sean Hobeck
 * @date 2025-08-06
 *
 * @file commit.h
 *    the commit module in the version control system, it is responsible for handling
 *    diffs and commits, as well as writing them to disk.
 */
#include "commit.h"

/*! @uses mkdir, getcwd. */
#include <sys/stat.h>

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*! @uses malloc, free. */
#include <stdlib.h>

/*! @uses time_t, struct tm, time, localtime, strftime. */
#include <time.h>

/*! @uses strcpy. */
#include <string.h>

/*! @uses pvc_t, pvc_collect, e_pvc_type_t. */
#include "pvc.h"

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @param branch_name the name of the parent branch that this commit is being placed under.
 * @return a commit_t structure containing the commit information.
 */
commit_t* commit_create(const char* message, char* branch_name) {
    // create a new commit structure.
    commit_t* commit = calloc(1, sizeof *commit);

    // set the commit message and branch name.
    commit->message = calloc(1, strlen(message) + 1);
    strcpy(commit->message, message);

    // grab the timestamp as well.
    time_t raw_time;
    struct tm *local_time;
    time(&raw_time);
    local_time = localtime(&raw_time);
    commit->timestamp = calloc(1, 21);
    strftime(commit->timestamp, 21, "%Y-%m-%d %H:%M:%S", local_time);

    // calculate the sha1 hash of the commit message, timestamp, random pointer, and a count of diffs.
    unsigned char* data = calloc(1, 2048);
    snprintf((char*)data, 2048, "%p %s%s %lu", &commit->changes,
        commit->message, commit->timestamp, commit->count);
    sha1(data, 2048, commit->hash);
    free(data);

    // create a directory under our current branch, store all of our diffs
    // in there as well as a commit file.
    char* path = calloc(1, 512);
    sprintf(path, ".lit/%s/commit-%lu-%s/", branch_name, raw_time, strsha1(commit->hash));
    if (mkdir(path, 0755) == -1) {
        perror("mkdir failed; could not create commit directory.\n");
        exit(-1); // exit on failure.
    }
    commit->path = calloc(1, strlen(path) + 1);
    strcpy(commit->path, path);
    free(path);

    // return our new commit.
    return commit;
};

/**
 * @brief add a diff to the commit, this will be used to store
 *  information about changes made to files or directories.
 *
 * @param commit the commit_t structure to which the diff will be added.
 * @param diff the diff_t structure containing the differences to be added.
 */
void commit_add_diff(commit_t* commit, diff_t* diff) {
    // if our count is greater than the capacity, call realloc.
    if (commit->count >= commit->capacity) {
        commit->capacity = commit->capacity ? commit->capacity * 2 : 1; // increment the diff count.
        diff_t** array = realloc(commit->changes, sizeof(diff_t*) * commit->capacity);
        if (!array) {
            perror("realloc failed; could not allocate memory for commit diffs.\n");
            exit(-1); // exit on failure.
        }
        commit->changes = array;
    }
    commit->changes[commit->count++] = diff; // copy the diff into the commit.
};

/**
 * @brief write the commit to a file in our '.lit' directory under our current branch.
 *
 * @param commit the commit_t structure to to be written to a file.
 */
void commit_write(const commit_t* commit) {
    // gather all of our diffs, and then write them to their respective files.
    for (size_t i = 0; i < commit->count; i++) {
        char* diff_path = calloc(1, 512);
        sprintf(diff_path, "%s%u.diff", commit->path, commit->changes[i]->hash);
        diff_write(commit->changes[i], diff_path);
        free(diff_path);
    }

    // now we write the commit information itself.
    char* commit_file = calloc(1, 512);
    sprintf(commit_file, "%scommit", commit->path);
    FILE* f = fopen(commit_file, "w");
    if (!f) {
        perror("fopen failed; could not open commit file for writing.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }
    free(commit_file);

    // write the commit information to the file.
    fprintf(f, "message:%s\ntimestamp:%s\nsha1:%s\nn_diffs:%lu\n", \
        commit->message, commit->timestamp, strsha1(commit->hash), commit->count);
    fclose(f);
};

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t* commit_read(const char* path) {
    // create a temporary commit structure.
    commit_t* commit = calloc(1 , sizeof *commit);

    // open the file for reading.
    char* filepath = calloc(1, 512);
    sprintf(filepath, "%s/commit", path);
    FILE* f = fopen(filepath, "r");
    if (!f) {
        perror("fopen failed; could not open commit file for reading.\n");
        exit(-1); // return an empty commit on failure.
    }
    free(filepath);

    // read the commit information from the file.
    char* hash = calloc(1, 41);
    commit->message = calloc(1, 1024);
    commit->timestamp = calloc(1, 80);
    size_t count = 0;
    int scanned = fscanf(f, "message:%1023[^\n]\ntimestamp:%79[^\n]\nsha1:%40[^\n]\nn_diffs:%lu\n", \
        commit->message, commit->timestamp, hash, &count);
    fclose(f);
    if (scanned != 4) {
        perror("fscanf failed; could not read commit header.\n");
        exit(-1); // exit with -1 on failure.
    }

    // this needs to be reversed into a character list based on the values of each char.
    unsigned char* _hash = strtoha(hash, 20);
    memcpy(commit->hash, _hash, 20);
    free(_hash);

    // no recurse because we are not snapshotting.
    commit->path = calloc(1, strlen(path) + 1);
    snprintf(commit->path, strlen(path), "%s\0", path);
    pvc_t* vector = pvc_collect(commit->path, E_PVC_TYPE_NO_RECURSE);
    if (!vector->nodes) {
        perror("pvc_collect failed; could not collect diffs from commit path.\n");
        exit(-1); // return an empty commit on failure.
    }

    // iterate through the collected files and read the diffs.
    for (size_t i = 0u; i < vector->count; i++) {
        pvc_inode_t* inode = vector->nodes[i];
        if (inode->type != E_PVC_INODE_TYPE_FILE || strcmp(inode->name, "commit") == 0) {
            // skip non-diff files.
            free(inode->path);
            free(inode->name);
            free(inode);
            continue;
        }

        // read the diff file.
        diff_t *diff = diff_read(inode->path);
        free(inode->path);
        free(inode->name);
        free(inode);

        // if the diff is not valid, skip it.
        if (diff->type == E_DIFF_TYPE_NONE) {
            continue;
        }
        commit_add_diff(commit, diff);
    }
    free(vector);
    return commit;
};