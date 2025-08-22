/**
 * @author Sean Hobeck
 * @date 2025-08-12
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

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @param branch_name the name of the parent branch that this commit is being placed under.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
create_commit(const char* message, char* branch_name) {
    // create a new commit structure.
    commit_t* commit = calloc(1, sizeof *commit);

    // set the commit message and branch name.
    commit->message = calloc(1, strlen(message) + 1);
    strcpy(commit->message, message);

    // grab the timestamp as well.
    commit->rawtime;
    struct tm *local_time;
    time(&commit->rawtime);
    local_time = localtime(&commit->rawtime);
    commit->timestamp = calloc(1, 21);
    strftime(commit->timestamp, 21, "%Y-%m-%d %H:%M:%S", local_time);

    // calculate the sha1 hash of the commit message, timestamp, random pointer, and a count of diffs.
    unsigned char* data = calloc(1, 2048);
    snprintf((char*)data, 2048, "commit\nmessage=%s\ntimestamp=%s\ndiff count=%lu\nrawtime=%lu",
        commit->message, commit->timestamp, commit->count, commit->rawtime);
    sha1(data, 128, commit->hash);
    free(data);

    // create a directory under our current branch, store all of our diffs
    // in there as well as a commit file.
    char* path = calloc(1, 256);
    char* hash = strsha1(commit->hash);
    sprintf(path, ".lit/objects/commits/%.2s/", hash);
    mkdir(path, 0755);
    sprintf(path, ".lit/objects/commits/%.2s/%s", hash, hash + 2);
    commit->path = calloc(1, strlen(path) + 1);
    strcpy(commit->path, path);
    free(path);
    free(hash);

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
void
add_diff_commit(commit_t* commit, diff_t* diff) {
    // if our count is greater than the capacity, call realloc.
    if (commit->count >= commit->capacity) {
        commit->capacity = commit->capacity ? commit->capacity * 2 : 1; // increment the diff count.
        diff_t** array = realloc(commit->changes, sizeof(diff_t*) * commit->capacity);
        if (!array) {
            fprintf(stderr,"realloc failed; could not allocate memory for commit diffs.\n");
            exit(EXIT_FAILURE); // exit on failure.
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
void
write_commit(const commit_t* commit) {
    // gather all of our diffs, and then write them to their respective files.
    for (size_t i = 0; i < commit->count; i++) {
        // for our content accessible storage we split the upper and lower into two ints.
        char* path = calloc(1, 256), *hash = calloc(1, 128);
        snprintf(hash, 128, "%04u", commit->changes[i]->crc);
        snprintf(path, 256, ".lit/objects/diffs/%.2s", hash);

        // ensure that directory exists.
        mkdir(path, 0755);
        snprintf(path, 256, ".lit/objects/diffs/%.2s/%s", hash, hash + 2);
        write_diff(commit->changes[i], path);
        free(hash);
        free(path);
    }

    // now we write the commit information itself.
    FILE* f = fopen(commit->path, "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open commit file for writing.\n");
        fclose(f);
        exit(EXIT_FAILURE); // exit on failure.
    }

    // write the commit information to the file.
    fprintf(f, "message:%s\ntimestamp:%s\nsha1:%s\ncount:%lu\nrawtime:%lu\n", \
        commit->message, commit->timestamp, strsha1(commit->hash), commit->count, commit->rawtime);

    // for each diff in this commit, write out the respective crc32 hash.
    for (size_t i = 0; i < commit->count; i++)
        fprintf(f, "%u\n", commit->changes[i]->crc);

    fclose(f);
};

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
read_commit(const char* path) {
    // create a temporary commit structure.
    commit_t* commit = calloc(1 , sizeof *commit);

    // open the file for reading.
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open commit file for reading.\n");
        exit(EXIT_FAILURE);
    }

    // read the commit information from the file.
    char* hash = calloc(1, 40);
    commit->message = calloc(1, 1024);
    commit->timestamp = calloc(1, 80);
    size_t count = 0;
    int scanned = fscanf(f, "message:%1024[^\n]\ntimestamp:%80[^\n]\nsha1:%40[^\n]\n"
        "count:%lu\nrawtime:%lu\n",commit->message, commit->timestamp, hash, &count, &commit->rawtime);
    if (scanned != 5) {
        fprintf(stderr,"fscanf failed; could not read commit header.\n");
        exit(EXIT_FAILURE); // exit with -1 on failure.
    }
    commit->path = strdup(path);

    // this needs to be reversed into a character list based on the values of each char.
    unsigned char* _hash = strtoha(hash, 20);
    memcpy(commit->hash, _hash, 20);
    free(_hash);

    // start reading the file for the count of diffs, then use
    //  the first two chars of the crc32 as the folder and then the
    //  for the file name in .lit/objects/diffs/xx/xxxx...
    for (size_t i = 0; i < count; i++) {
        ucrc32_t crc = 0;
        fscanf(f, "%u\n", &crc);

        // construct the file location from the hash.
        char* dpath = calloc(1, 256), *dhash = calloc(1, 128);
        snprintf(dhash, 128, "%04u", crc);
        snprintf(dpath, 256, ".lit/objects/diffs/%.2s/%s", dhash, dhash + 2);
        diff_t* diff = read_diff(dpath);
        add_diff_commit(commit, diff);
        free(dhash);
        free(dpath);
    }
    fclose(f);
    return commit;
};