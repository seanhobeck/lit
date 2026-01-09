/**
 * @author Sean Hobeck
 * @date 2026-01-09
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
    commit->timestamp = calloc(1, TIMESTAMP_MAX_CHARS + 1);
    strftime(commit->timestamp, TIMESTAMP_MAX_CHARS + 1, \
        "%Y-%m-%d %H:%M:%S", local_time);

    /* calculate the sha1 hash based on some random metrics; todo: change this. */
    unsigned char* data = calloc(1, PATH_MAX);
    snprintf((char*)data, PATH_MAX,
        "commit\n"
        "message=%s\n"
        "timestamp=%s\n"
        "diff count=%p\n"
        "rawtime=%lu",
        commit->message,
        commit->timestamp,
        &commit->changes,
        commit->rawtime);
    sha1(data, 128, commit->hash);
    free(data);

    /* create a directory under our current branch, store all of our diffs
     * in there as well as a commit file. */
    char* path = calloc(1, PATH_MAX);
    char* hash = strsha1(commit->hash);
    sprintf(path, ".lit/objects/commits/%.2s/", hash);
    mkdir(path, MKDIR_MOWNER);
    sprintf(path, ".lit/objects/commits/%.2s/%s", hash, hash + 2);
    free(hash);

    /* copy over the path. */
    commit->path = strdup(path);
    free(path);

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
        char *hash = strcrc32(change->crc);
        char* path = calloc(1, PATH_MAX);
        snprintf(path, PATH_MAX, ".lit/objects/diffs/%.2s", hash);

        /* ensure that directory exists. */
        mkdir(path, MKDIR_MOWNER);
        snprintf(path, PATH_MAX, ".lit/objects/diffs/%.2s/%s", hash, hash + 2);
        free(hash);

        /* write the diff and free the path. */
        write_diff(change, path);
        free(path);
    _endforeach;

    /* now we write the commit information itself. */
    FILE* stream = fopen(commit->path, "w");
    if (!stream) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open commit file for writing.\n");
        fclose(stream);
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* call to write everything. */
    write_commit_to_stream(stream, commit);
    fclose(stream);
}

/**
 * @brief write the commit pointer to a generic open file stream.
 *
 * @param stream the stream from fopen() on some file descriptor.
 * @param commit the commit to be written to the stream.
 */
void
write_commit_to_stream(FILE* stream, const commit_t* commit) {
    /* write the commit information to the file. */
    fprintf(stream, "message:%s\ntimestamp:%s\nsha1:%s\ncount:%zu\nrawtime:%zu\n", \
        commit->message, commit->timestamp, strsha1(commit->hash), commit->changes->length,
        commit->rawtime);

    /* for each diff in this commit, write out the respective crc32 hash. */
    _foreach(commit->changes, const diff_t*, change)
        fprintf(stream, "%u\n", change->crc);
    _endforeach;
}

/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t*
read_commit(const char* path) {
    /* open the file for reading. */
    FILE* stream = fopen(path, "r");
    if (!stream) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open commit file for reading.\n");
        exit(EXIT_FAILURE);
    }

    /* we simply call the function. */
    commit_t* result = read_commit_from_stream(stream);
    result->path = strdup(path);
    fclose(stream);
    return result;
}

/**
 * @brief read a commit from a generic open file stream.
 *
 * @param stream the stream from which to read from.
 * @return a pointer to an allocated commit with all the data.
 */
commit_t*
read_commit_from_stream(FILE* stream) {
    /* create a partial commit structure. */
    commit_t* commit = calloc(1 , sizeof *commit);

    /* create the dynamic array. */
    commit->changes = dyna_create();

    /* read the commit information from the file. */
    char* hash = calloc(1, SHA1_MAX_CHARS + 1);
    commit->message = calloc(1, MESSAGE_MAX_CHARS + 1);
    commit->timestamp = calloc(1, TIMESTAMP_MAX_CHARS + 1);
    char rawtime_string[32 + 1], count_string[32 + 1]; /* plus null term. */
    int scanned = fscanf(stream,
        "message:%8192[^\n]\n"
        "timestamp:%80[^\n]\n"
        "sha1:%40[^\n]\n"
        "count:%32[0-9]\n"
        "rawtime:%32[0-9]\n",
        commit->message,
        commit->timestamp,
        hash,
        count_string,
        rawtime_string);
    if (scanned != 5) {
        llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read commit header.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* conversion using sstrtosz. */
    commit->rawtime = (long) sstrtosz(rawtime_string);
    size_t count = sstrtosz(count_string);

    /* this needs to be reversed into a character list based on the values of each char. */
    unsigned char* tmp_hash = strtoha(hash, SHA1_SIZE);
    memcpy(commit->hash, tmp_hash, SHA1_SIZE);
    free(tmp_hash);
    free(hash);

    /* then we read all the changes respectively. */
    for (size_t i = 0; i < count; i++) {
        /* scan each line. */
        char* line = calloc(1, LINE_MAX_CHARS + 1);
        if (fscanf(stream, "%256[^\n]\n", line) != 1) {
            llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read diff hash.\n");
            exit(EXIT_FAILURE); /* exit on failure. */
        }

        /* construct the file location from the hash. */
        char* filepath = calloc(1, PATH_MAX);
        snprintf(filepath, PATH_MAX, ".lit/objects/diffs/%.2s/%s", line, line + 2);
        diff_t* diff = read_diff(filepath);

        /* push to the dynamic array. */
        dyna_push(commit->changes, diff);
        free(filepath);
        free(line);
    }
    return commit;
}