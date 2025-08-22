/**
 * @author Sean Hobeck
 * @date 2025-08-12
 *
 * @file ops.c
 *    the branch module in the version control system, it is responsible for handling
 *    commits and applying operations between branches
 */
#include "ops.h"

/*! @uses fopen, FILE*, fclose, fprintf */
#include <stdio.h>

/*! @uses malloc, free */
#include <stdlib.h>

/*! @uses strcmp */
#include <string.h>

/*! @uses mkdir */
#include <sys/stat.h>

/**
 * @brief free lines of <n> size.
 *
 * @param lines the ptr to the lines array in memory.
 * @param n the count/capacity of <lines> as an array.
 */
void
freels(char** lines, size_t n) {
    for (size_t i = 0; i < n; i++)
        free(lines[i]);
    free(lines);
};

/**
 * @brief given some lines from a lcs algorithm, extract only the original lines
 *  (keep ' ' and '-', ignore '+') for inverse application.
 *
 * @param lines the diff lines
 * @param n the number of lines in the diff
 * @param k pointer to store the resulting count
 * @return allocated array of cleaned lines
 */
char**
finversels(char** lines, size_t n, size_t* k) {
    // allocate the cleaned lines
    char** clines = calloc(1, n * sizeof(char*));

    // iterate...
    size_t m = 0;
    for (size_t i = 0; i < n; i++) {
        char* line = lines[i];

        // keep unchanged lines and removed lines (for restore)
        if (line[0] == ' ') clines[m++] = strdup(line + 1);
        else if (line[0] == '-') clines[m++] = strdup(line + 2);
        else if (line[0] != '+') clines[m++] = strdup(line);
        // ignore '+' lines (these were additions we want to remove)
    }
    *k = m;
    return clines;
}

/**
 * @brief given some lines from a lcs algorithm, extract only the original lines
 *  (keep ' ' and '+', ignore '-') for forward application.
 *
 * @param lines the diff lines
 * @param n the number of lines in the diff
 * @param k pointer to store the resulting count
 * @return allocated array of cleaned lines
 */
char**
fforwardls(char** lines, size_t n, size_t* k) {
    // allocate the cleaned lines
    char** clines = calloc(1, n * sizeof(char*));

    // iterate...
    size_t m = 0;
    for (size_t i = 0; i < n; i++) {
        char* line = lines[i];

        // keep unchanged lines and removed lines (for restore)
        if (line[0] == ' ') clines[m++] = strdup(line + 1);
        else if (line[0] == '+') clines[m++] = strdup(line + 2);
        else if (line[0] != '-') clines[m++] = strdup(line);
        // ignore '-' lines (these were additions we want to remove)
    }
    *k = m;
    return clines;
}

/**
 * @brief apply the commit forward to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
apply_forward_commit(const commit_t* commit) {
    // iterate for a "delta apply".
    for (size_t i = 0; i < commit->count; i++) {
        diff_t* diff = commit->changes[i];
        switch (diff->type) {
            case (E_DIFF_FILE_NEW): {
                // write this out to the file.
                size_t n = 0;
                char** lines = fforwardls(diff->lines, diff->count, &n);
                fwritels(diff->new_path, lines, n);
                freels(lines, n);
                break;
            }
            case (E_DIFF_FILE_MODIFIED): {
                if (strcmp(diff->new_path, diff->stored_path) != 0) {
                    // if the file was renamed, we need to remove the old file.
                    remove(diff->stored_path);
                }

                // write this out to the new file.
                size_t n = 0;
                char** lines = fforwardls(diff->lines, diff->count, &n);
                fwritels(diff->new_path, lines, n);
                freels(lines, n);
                break;
            }
            case (E_DIFF_FOLDER_NEW): {
                // make a new folder.
                mkdir(diff->stored_path, 0755);
                break;
            }
                // if a folder was deleted
            case (E_DIFF_FILE_DELETED):
            case (E_DIFF_FOLDER_DELETED): {
                // then we 'unlink' this folder.
                remove(diff->stored_path);
                break;
            }
            default: ; /// ?
        }
    }
};

/**
 * @brief apply the commit backwards (inverse) to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
apply_inverse_commit(const commit_t* commit) {
    // iterate for a "delta apply".
    for (size_t i = 0; i < commit->count; i++) {
        diff_t* diff = commit->changes[i];
        switch (diff->type) {
            case (E_DIFF_FOLDER_NEW):
            case (E_DIFF_FILE_NEW): {
                // file / folder was created so delete it.
                remove(diff->stored_path);
                break;
            }
            case (E_DIFF_FILE_MODIFIED): {
                if (strcmp(diff->new_path, diff->stored_path) != 0) {
                    // if the file was renamed, we need to remove the old file.
                    remove(diff->new_path);
                }

                // write this out to the new file.
                size_t n = 0;
                char** lines = finversels(diff->lines, diff->count, &n);
                fwritels(diff->stored_path, lines, n);
                freels(lines, n);
                break;
            }
            case (E_DIFF_FOLDER_DELETED): {
                // make a new folder.
                mkdir(diff->stored_path, 0755);
                break;
            }
            case (E_DIFF_FILE_DELETED): {
                // write this out to the file.
                size_t n = 0;
                char** lines = finversels(diff->lines, diff->count, &n);
                fwritels(diff->stored_path, lines, n);
                freels(lines, n);
                break;
            }
            default: ; /// ?
        }
    }
};

/**
 * @brief rollback to a older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
rollback(branch_t* branch, const commit_t* commit) {
    // first thing to do is to check that this commit is in <branch> history.
    size_t target_idx = (size_t) -1;
    for (size_t i = 0; i < branch->count; i++) {
        // we compare by hashes, not by pointers.
        if (!strcmp(branch->commits[i]->hash, commit->hash)) {
            target_idx = i;
            break;
        }
    }

    // if the commit is not in the history, report the error and return.
    if (target_idx == -1) {
        fprintf(stderr,"index == -1; commit not found in branch history.\n");
        return;
    }

    // apply inverse of commits from current back to target
    //      go backwards from current position to target
    for (size_t i = branch->idx; i > target_idx; i--) {
        apply_inverse_commit(branch->commits[i]);
    }
    branch->idx = target_idx;
};

/**
 * @brief checkout to a newer commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
checkout(branch_t* branch, const commit_t* commit) {
    // first thing to do is to check that this commit is in <branch> history.
    size_t target_idx = -1;
    for (size_t i = 0; i < branch->count; i++) {
        // we compare by hashes, not by pointers.
        if (!strcmp(branch->commits[i]->hash, commit->hash)) {
            target_idx = i;
            break;
        }
    }

    // if the commit is not in the history, report the error and return.
    if (target_idx == -1) {
        fprintf(stderr,"index == -1; commit not found in branch history.\n");
        exit(EXIT_FAILURE);
    }

    // apply commits from current forward to target
    //      go forwards from current position to target
    for (size_t i = branch->idx + 1; i <= target_idx; i++) {
        apply_forward_commit(branch->commits[i]);
    }
    branch->idx = target_idx;
};