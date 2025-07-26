/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file branch.c
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#include <branch.h>

/*! @uses strncpy. */
#include <string.h>

/*! @uses mkdir, getcwd. */
#include <sys/types.h>
#include <sys/stat.h>

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*!@ uses malloc, free. */
#include <stdlib.h>

/*! @uses sha1_t, sha1, sha256_t, sha256. */
#include <hash.h>

/*! @uses pvc_t, pvc_collect. */
#include <pvc.h>

/**
* @brief create a new branch with the given name.
*
* @param name the name of the branch to be created.
* @return a branch_t structure containing the branch information.
*/
branch_t branch_create(const char* name) {
    // create a new branch structure.
    branch_t branch = {
        .name = {0},
        .path = {0},
        .hash = {0},
        .history = {0},
        .current_commit = 0u,
        .current_commit_hash = {0},
    };

    // copy the name into the branch structure.
    strncpy(branch.name, name, sizeof(branch.name) - 1);

    // create the branch path based on the cwd.
    char path[256u];
    sprintf(path, ".lit/%s/", branch.name);
    if (mkdir(path, 0755) == -1) {
        perror("mkdir failed; could not create branch directory.\n");
        return branch;
    }
    // copy the branch path into the branch structure.
    strncpy(branch.path, path, 255u);

    // & calculate the sha256 hash of the branch name.
    sha256(branch.name, strlen(branch.name), branch.hash);

    // write out to the file and return.
    branch_write(&branch);
    return branch;
};
/**
 * @brief create a folder to hold all of the branch's commits and diffs.
 *
 * @param branch the branch_t structure to be written to a file.
 */
void branch_write(const branch_t* branch) {
    // create branch file.
    char path[256u];
    sprintf(path, "%s/branch.txt", branch->path);
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("fopen failed; could not open branch file for writing.\n");
        exit(-1); // exit on failure.
    }

    // write the branch name and hash to the file.
    fprintf(f, "name:%s\nsha256:%s\n", branch->name, strsha256(branch->hash));
    fprintf(f, "history_count:%lu\n", branch->current_commit);
    fprintf(f, "current_commit:%s\n", strsha1(branch->current_commit_hash));
    fclose(f);

    // write all of the commits in the branch history to their respective files.
    for (unsigned long i = 0u; i < branch->history.n_commits; i++) {
        commit_write(branch->history.commits[i]);
    }
};

/**
 * @brief add a commit to the branch history.
 *
 * @param commit the commit_t structure to be added to the branch history.
 * @param history the history_t structure to which the commit will be added.
 */
void branch_add_commit(commit_t* commit, history_t* history) {
    history->commits = realloc(history->commits, sizeof(commit_t*) * (history->n_commits + 1u));
    if (!history->commits) {
        perror("realloc failed; could not allocate memory for branch history commits.\n");
        exit(-1); // exit on failure.
    }
    // copy the commit into the history.
    history->commits[history->n_commits++] = commit;
};

/**
 * @brief read the commit history on a branch.
 *
 * @param path the path to the branch history file.
 * @return a history_t structure containing the commit history.
 */
history_t branch_read_history(const char* path) {
    // snapshot the current state of the branch path
    pvc_t vector = pvc_collect(path, E_PVC_TYPE_NO_RECURSE);

    // create the history for the wd.
    history_t history = {
        .n_commits = 0x0,
        .commits = 0x0,
    };
    for (unsigned long i = 0u; i < vector.count; i++) {
        // if we are reading branch.txt, ignore it.
        if (strcmp(vector.nodes[i].name, "branch.txt") == 0) continue;
        if (strcmp(vector.nodes[i].name, ".") == 0) continue;

        // read a commit from the vector.
        commit_t commit = commit_read(vector.nodes[i].path);
        commit_t *heap_commit = malloc(sizeof *heap_commit);
        if (!heap_commit) {
            perror("malloc failed; could not allocate memory for commit.\n");
            exit(1);
        }
        *heap_commit = commit; // copy all fields
        branch_add_commit(heap_commit, &history);
    }
    return history;
};

/**
 * @brief read a branch from a file in our '.lit' directory.
 *
 * @param name the name of our branch.
 * @return a branch_t structure containing the branch information.
 */
branch_t branch_read(const char* name) {
    // create a temporary branch structure.
    branch_t branch = {
        .name = {0},
        .path = {0},
        .hash = {0},
        .history = {0},
        .current_commit = 0u,
        .current_commit_hash = {0},
    };
    // create the branch path based on the cwd.
    char path[256u];
    sprintf(path, ".lit/%s", name);
    strncpy(branch.path, path, 255u);
    sprintf(path, "%s/branch.txt", branch.path);

    // open the branch file for reading.
    FILE* f = fopen(path, "r");
    if (!f) {
        perror("fopen failed; could not open branch file for reading.\n");
        exit(-1); // exit on failure.
    }
    // read the branch information from the file.
    strncpy(branch.name, name, sizeof(branch.name) - 1);
    char dname[128u], branch_hash[65u], branch_current_commit[41u];
    unsigned long current_commit = 0u;
    int scanned = fscanf(f, "name:%127[^\n]\nsha256:%64[^\n]\n"
                            "history_count:%lu\ncurrent_commit:%40[^\n]\n", \
        dname, branch_hash, &current_commit, branch_current_commit);
    if (scanned != 4) {
        perror("fscanf failed; could not read branch header.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }
    // this needs to be reversed into a character list based on the values of each char.
    for (unsigned long i = 0u; i < 32u; i++) {
        unsigned char byte;
        sscanf(branch_hash + i * 2u, "%02x", &byte);
        branch.hash[i] = (char) byte;
    }
    for (unsigned long i = 0u; i < 20u; i++) {
        unsigned char byte;
        sscanf(branch_current_commit + i * 2u, "%02x", &byte);
        branch.current_commit_hash[i] = (char) byte;
    }
    branch.current_commit = current_commit;

    // read the history.
    branch.history = branch_read_history(branch.path);

    // cleanup
    fclose(f);
    return branch;
};