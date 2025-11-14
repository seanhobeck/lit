/**
 * @author Sean Hobeck
 * @date 2025-11-12
 *
 * @file ops.c
 *    the operations module, it is responsible for handling
 *    commits and applying operations between branches.
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
 * @brief apply the commit forward to the files currently existing.
 *
 * @param commit the commit to be applied forward.
 */
void
forward_commit_op(const commit_t* commit) {
    // assert on the commit.
    assert(commit != 0x0);

    // iterate for a "delta apply".
    for (size_t i = 0; i < commit->count; i++) {
        diff_t* diff = commit->changes[i];
        switch (diff->type) {
            case (E_DIFF_FILE_NEW): {
                // write this out to the file.
                size_t n = 0;
                char** lines = fforwardls(diff->lines, diff->count, &n);
                fwritels(diff->new_path, lines, n);
                ffreels(lines, n);
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
                ffreels(lines, n);
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
reverse_commit_op(const commit_t* commit) {
    // assert on the commit.
    assert(commit != 0x0);

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
                ffreels(lines, n);
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
                ffreels(lines, n);
                break;
            }
            default: ; /// ?
        }
    }
};

/**
 * @brief rollback to an older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
rollback_op(branch_t* branch, const commit_t* commit) {
    // assert on the branch and commit.
    assert(branch != 0x0);
    assert(commit != 0x0);

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
    for (size_t i = branch->head; i > target_idx; i--) {
        reverse_commit_op(branch->commits[i]);
    }
    branch->head = target_idx;
};

/**
 * @brief checkout to a newer commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void
checkout_op(branch_t* branch, const commit_t* commit) {
    // assert on the branch and the commit.
    assert(branch != 0x0);
    assert(commit != 0x0);

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

    // apply commits from current forward to target,
    //      go forwards from current position to target
    for (size_t i = branch->head + 1; i <= target_idx; i++) {
        forward_commit_op(branch->commits[i]);
    }
    branch->head = target_idx;
};