/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file commit.h
 *    the commit module in the version control system, it is responsible for handling
 *    diffs and commits, as well as writing them to disk.
 */
#include <commit.h>

/*! @uses mkdir, getcwd. */
#include <sys/types.h>
#include <sys/stat.h>

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*!@ uses malloc, free. */
#include <stdlib.h>

/*! @uses time_t, struct tm, time, localtime, strftime. */
#include <time.h>

/*! @uses strcpy. */
#include <string.h>

/*! @uses pvc_t, pvc_collect, e_pvc_type_t. */
#include <pvc.h>

/**
 * @brief create a new commit with the given message, snapshotting the current state
 *  of your working directory and storing the diffs in the commit.
 *
 * @param message the commit message.
 * @return a commit_t structure containing the commit information.
 */
commit_t commit_create(const char* message, char branch_name[128u]) {
    // create a new commit structure.
    commit_t commit = {
        .n_diffs = 0u,
        .diffs = 0x0,
        .hash = {0},
    };

    // set the commit message.
    strncpy(commit.message, message, 1024u);
    // grab the timestamp as well.
    time_t raw_time;
    struct tm* local_time;
    time(&raw_time);
    local_time = localtime(&raw_time);
    strftime(commit.timestamp, sizeof(commit.timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    // calculate the sha1 hash of the commit message, timestamp, random pointer, and a count of diffs.
    unsigned char data[2048u];
    snprintf((char*)data, sizeof(data), "%p %s%s %d", &commit.diffs, commit.message,
        commit.timestamp, commit.n_diffs);
    sha1(data, strlen((char*)data), commit.hash);

    // create a directory under our current branch, store all of our diffs
    // in there as well as a commit file.
    char commit_path[512u];
    sprintf(commit_path, ".lit/%s/commit-%s/", branch_name, strsha1(commit.hash));
    if (mkdir(commit_path, 0755) == -1) {
        perror("mkdir failed; could not create commit directory.\n");
        exit(-1); // exit on failure.
    }
    strncpy(commit.commit_path, commit_path, 511u);

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
    commit->diffs = realloc(commit->diffs, sizeof(diff_t*) * (commit->n_diffs + 1u));
    if (!commit->diffs) {
        perror("realloc failed; could not allocate memory for commit diffs.\n");
        exit(-1); // exit on failure.
    }
    commit->diffs[commit->n_diffs++] = diff; // copy the diff into the commit.
};

/**
 * @brief write the commit to a file in our '.lit' directory under our current branch.
 *
 * @param commit the commit_t structure to to be written to a file.
 */
void commit_write(const commit_t* commit) {
    // gather all of our diffs, and then write them to their respective files.
    for (unsigned long i = 0u; i < commit->n_diffs; i++) {
        char diff_path[512u];
        sprintf(diff_path, "%s%u.diff", commit->commit_path, commit->diffs[i]->hash);
        diff_write(commit->diffs[i], diff_path);
    }

    // now we write the commit information itself.
    char commit_file[512u];
    sprintf(commit_file, "%scommit.txt", commit->commit_path);
    FILE* f = fopen(commit_file, "w");
    if (!f) {
        perror("fopen failed; could not open commit file for writing.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }

    // write the commit information to the file.
    fprintf(f, "message:%s\ntimestamp:%s\nsha1:%s\nn_diffs:%lu\n", \
        commit->message, commit->timestamp, strsha1(commit->hash), commit->n_diffs);
    fclose(f);
};


/**
 * @brief read a commit from a file in our '.lit' directory under our current branch.
 *
 * @param path the path to the commit file.
 * @return a commit_t structure containing the commit information.
 */
commit_t commit_read(const char* path) {
    // create a temporary commit structure.
    commit_t commit = {
        .n_diffs = 0u,
        .diffs = 0x0,
        .hash = {0},
    };

    // open the file for reading.
    char npath[512u];
    sprintf(npath, "%s/", path);
    strncpy(commit.commit_path, npath, 511u);
    sprintf(npath, "%scommit.txt", commit.commit_path);
    FILE* f = fopen(npath, "r");
    if (!f) {
        perror("fopen failed; could not open commit file for reading.\n");
        exit(-1); // return an empty commit on failure.
    }

    // read the commit information from the file.
    char message[1024u], timestamp[80u], sha1_hash[41u];
    unsigned long n_diffs = 0u;
    int scanned = fscanf(f, "message:%1023[^\n]\ntimestamp:%79[^\n]\nsha1:%40[^\n]\nn_diffs:%lu\n", \
        message, timestamp, sha1_hash, &n_diffs);
    if (scanned != 4) {
        perror("fscanf failed; could not read commit header.\n");
        fclose(f);
        exit(-1); // exit with -1 on failure.
    }
    strncpy(commit.message, message, 1023u);
    strncpy(commit.timestamp, timestamp, 79u);
    fclose(f);

    // this needs to be reversed into a character list based on the values of each char.
    for (unsigned long i = 0u; i < 20u; i++) {
        unsigned char byte;
        sscanf(sha1_hash + i * 2u, "%02x", &byte);
        commit.hash[i] = (char) byte;
    }

    // no recurse because we are not snapshotting.
    pvc_t vector = pvc_collect(commit.commit_path, E_PVC_TYPE_NO_RECURSE);
    if (!vector.nodes) {
        perror("pvc_collect failed; could not collect diffs from commit path.\n");
        free(commit.diffs);
        exit(-1); // return an empty commit on failure.
    }

    // iterate through the collected files and read the diffs.
    for (unsigned long i = 0u; i < vector.count; i++) {
        pvc_inode_t inode = vector.nodes[i];
        if (inode.type != E_PVC_INODE_TYPE_FILE || strcmp(inode.name, "commit.txt") == 0) {
            // skip non-diff files.
            free(inode.path);
            free(inode.name);
            continue;
        }

        // read the diff file.
        diff_t diff = diff_read(inode.path);
        if (diff.type == E_DIFF_TYPE_NONE) {
            // if the diff is not valid, skip it.
            free(inode.path);
            free(inode.name);
            continue;
        }

        // add the diff to the commit.
        diff_t *heap_diff = malloc(sizeof *heap_diff);
        if (!heap_diff) {
            perror("malloc failed; could not allocate memory for heap diff.\n");
            exit(-1);
        }
        *heap_diff = diff; // copy the contents
        commit_add_diff(&commit, heap_diff);
    }
    return commit;
};