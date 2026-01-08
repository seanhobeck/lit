/**
 * @author Sean Hobeck
 * @date 2026-01-08
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

/*! @uses assert. */
#include <assert.h>

/*! @uses diff_t. */
#include "diff.h"

/*! @uses MKDIR_MOWNER. */
#include "utl.h"

/*! @uses llog, E_LOGGER_LEVEL_INFO. */
#include "log.h"

/*!~ @note this is a format for the main parts of data that are written at the start (header) of
 *  the file for a commit, stored within a branch, within the repository. */
#define COMMIT_HEADER_FORMAT "message:%1024[^\n]\ntimestamp:%80[^\n]\nsha1:%40[^\n]\ncount:%lu\nrawtime:%lu\n"

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @param branch_name the name of the parent branch that this commit is being placed under.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
create_commit(const char* message, const char* branch_name) {
    /* assert on the message and the branch name. */
    assert(message != 0x0);
    assert(branch_name != 0x0);

    /* create a new commit structure. */
    commit_t* commit = calloc(1, sizeof *commit);

    /* set the commit message and branch name. */
    commit->message = calloc(1, strlen(message) + 1);
    strcpy(commit->message, message);

    /* create the dynamic list. */
    commit->changes = dyna_create();

    /* grab the timestamp as well. */
    time(&commit->rawtime);
    struct tm *local_time = localtime(&commit->rawtime);
    commit->timestamp = calloc(1, 21);
    strftime(commit->timestamp, 21, "%Y-%m-%d %H:%M:%S", local_time);

    /* calculate the sha1 hash based on some random metrics; @TODO: change this. */
    unsigned char* data = calloc(1, 2049);
    snprintf((char*)data, 2049, "commit\nmessage=%s\ntimestamp=%s\ndiff count=%p\nrawtime=%lu",
        commit->message, commit->timestamp, &commit->changes, commit->rawtime);
    sha1(data, 128, commit->hash);
    free(data);

    /* create a directory under our current branch, store all of our diffs
     * in there as well as a commit file. */
    char* path = calloc(1, 257);
    char* hash = strsha1(commit->hash);
    snprintf(path, 256, ".lit/objects/commits/%.2s/", hash);
    mkdir(path, MKDIR_MOWNER);
    snprintf(path, 256, ".lit/objects/commits/%.2s/%s", hash, hash + 2);
    commit->path = calloc(1, 257);
    strncpy(commit->path, path, 256);
    free(path);
    free(hash);

    /* return our new commit. */
    return commit;
}

/**
 * @brief write the commit to a file in our '.lit' directory under our current branch.
 *
 * @param commit the commit_t structure to be written to a file.
 */
void
write_commit(const commit_t* commit) {
    /* assert on the commit ptr. */
    assert(commit != 0x0);

    /* gather all changes and write them to their respective files. */
    _foreach(commit->changes, const diff_t*, change)
        /* for our content-accessible storage we split the upper and lower into two ints. */
        char* path = calloc(1, 257), *hash = calloc(1, 129);
        snprintf(hash, 128, "%04u", change->crc);
        snprintf(path, 256, ".lit/objects/diffs/%.2s", hash);

        /* ensure that directory exists. */
        mkdir(path, MKDIR_MOWNER);
        snprintf(path, 256, ".lit/objects/diffs/%.2s/%s", hash, hash + 2);
        write_diff(change, path);
        free(hash);
        free(path);
    _endforeach;

    /* now we write the commit information itself. */
    FILE* f = fopen(commit->path, "w");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open commit file for writing.\n");
        fclose(f);
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* write the commit information to the file. */
    fprintf(f, "message:%s\ntimestamp:%s\nsha1:%s\ncount:%lu\nrawtime:%lu\n", \
        commit->message, commit->timestamp, strsha1(commit->hash), commit->changes->length,
        commit->rawtime);

    /* for each diff in this commit, write out the respective crc32 hash. */
    _foreach(commit->changes, const diff_t*, change)
        fprintf(f, "%u\n", change->crc);
    _endforeach;
    fclose(f);
}

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
read_commit(const char* path) {
    /* create a temporary commit structure. */
    commit_t* commit = calloc(1 , sizeof *commit);

    /* open the file for reading. */
    FILE* f = fopen(path, "r");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open commit file for reading.\n");
        exit(EXIT_FAILURE);
    }

    /* create the dynamic array. */
    commit->changes = dyna_create();

    /* read the commit information from the file. */
    char* hash = calloc(1, 41);
    commit->message = calloc(1, 1025);
    commit->timestamp = calloc(1, 81);
    size_t count = 0;
    int scanned = fscanf(f, COMMIT_HEADER_FORMAT, \
        commit->message, commit->timestamp, hash, &count, &commit->rawtime);
    if (scanned != 5) {
        llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read commit header.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }
    commit->path = strdup(path);

    /* this needs to be reversed into a character list based on the values of each char. */
    unsigned char* _hash = strtoha(hash, 20);
    memcpy(commit->hash, _hash, 20);
    free(_hash);
    free(hash);

    /* start reading the file for the count of diffs, then use
     *  the first two chars of the crc32 as the folder and then the
     *  for the file name in .lit/objects/diffs/xx/xxxx...  */
    for (size_t i = 0; i < count; i++) {
        /* changed to strtoul for clang-tidy purposes. */
        char* line = calloc(1, 129);
        fscanf(f, "%128[^\n]\n", line);
        ucrc32_t crc = strtoul(line, 0x0, 10);
        free(line);

        /* construct the file location from the hash. */
        char* filepath = calloc(1, 257), *new_hash = calloc(1, 129);
        snprintf(new_hash, 128, "%04u", crc);
        snprintf(filepath, 256, ".lit/objects/diffs/%.2s/%s", new_hash, new_hash + 2);
        diff_t* diff = read_diff(filepath);

        /* push to the dynamic array. */
        dyna_push(commit->changes, diff);
        free(new_hash);
        free(filepath);
    }

    /* close the file and return. */
    fclose(f);
    return commit;
}