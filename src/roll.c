/**
 * @author Sean Hobeck
 * @date 2025-08-07
 *
 * @file roll.c
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#include "roll.h"

/*! @uses fopen, FILE*, fclose, fprintf */
#include <stdio.h>

/*! @uses malloc, free */
#include <stdlib.h>

/*! @uses mkdir */
#include <sys/stat.h>

/*! @uses strcmp */
#include <string.h>

/*! @uses errno, EEXIST */
#include <errno.h>

/**
 * @brief ensure all directories in the given path (up to the final slash) exist.
 *
 * @param path the full path.
 * @return 0 on success, -1 on error.
 */
int ensure_parent_dirs(const char *path) {
    // string duplicate the path to avoid modifying the original.
    char *dup = strdup(path);
    if (!dup) return -1;

    // iterate through each character in the path.
    for (char *p = dup + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            // check if the directory exists, if not, create it.
            if (mkdir(dup, 0755) != 0 && errno != EEXIST) {
                free(dup);
                return -1;
            }
            *p = '/';
        }
    }
    free(dup);
    return 0;
}

/**
 * @brief write rollback lines to a file path provided.
 *
 * @param path the path to the file to be created/written to.
 * @param lines the allocated number of lines from rollback_diff.
 * @param n the number of lines in the rolled-back diff.
 */
void write_lines(const char* path, char** lines, int n) {
    // ensure that the parent directories exist on <path>.
    if (ensure_parent_dirs(path) == -1) {
        fprintf(stderr,"could not create parent directories.\n");
        exit(-1); // exit on failure.
    }

    // open the file.
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open file for rollback writing.\n");
        return;
    }

    // iterate over the lines and use fprintf();
    for (size_t i = 0; i < n; i++) {
        fprintf(f, "%s\n", lines[i]);
    }
    fclose(f);
};

/**
 * @brief search through a commit to find if a name is in any of the diffs.
 *
 * @param commit the commit to search in.
 * @param name the name to be searched for.
 * @return if <name> was found in <commit>'s diffs (-1 -> no, > 0 yes).
 */
size_t search_commit_name(const commit_t* commit, const char* name) {
    // iterate over each diff in the commit.
    for (size_t i = 0; i < commit->count; i++)
        if (!strcmp(commit->changes[i]->new_path, name) ||
            !strcmp(commit->changes[i]->stored_path, name))
            return i;
    return -1;
}

/**
 * @brief given a older diff, rollback to it (ignore anything -, keep ' ' and +)
 *
 * @param diff the diff provided (from a older commit).
 * @param n the number of lines in the rolled-back diff.
 * @return a allocated number of lines strdup'd.
 */
char ** rollback_to_diff(const diff_t* diff, int* n) {
    // allocate the lines
    char** lines = calloc(1, diff->count * sizeof(char*));

    // iterate...
    int m = 0;
    for (size_t i = 0; i < diff->count; i++) {
        char* line = diff->lines[i];

        // if there is a space, there isn't a two char prefix.
        if (line[0] == ' ')
            lines[m++] = strdup(line+1u);
        else if (line[0] == '+') {
            // skip the two characters '+ '
            lines[m++] = strdup(line+2u);
        }
        else if (line[0] != '-')
            lines[m++] = strdup(line);
    }
    *n = m;
    return lines;
};

/**
 * @brief rollback to a older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void rollback(branch_t* branch, const commit_t* commit) {
    // find the differences between <branch>'s commit and <commit>.
    commit_t* current = branch->commits[branch->idx-1];

    // first thing to do is to check that this commit is in <branch> history.
    long idx = -1;
    for (size_t i = 0; i < branch->count; i++) {
        // we compare by hashes, not by pointers.
        if (!strcmp(branch->commits[i]->hash, commit->hash)) {
            idx = i;
            break;
        }
    }

    // if the commit is not in the history, report the error and return.
    if (idx == -1) {
        fprintf(stderr,"commit not found in branch history.\n");
        return;
    }

    // iterate for a "delta apply".
    for (size_t i = 0; i < current->count; i++) {
        diff_t* diff = current->changes[i];
        switch (diff->type) {
            // if a file was created and is not in our commit, delete it.
            case (E_DIFF_NEW_FILE): {
                // then we 'unlink' this diff.
                remove(diff->stored_path);
                break;
            }
            // if a file was deleted and is in our commit, restore it.
            case (E_DIFF_FILE_DELETED): {
                size_t index = search_commit_name(commit, diff->stored_path);
                if (index == -1) {
                    break;
                }

                // write this out to the file.
                int n = 0;
                diff_t* restore = commit->changes[index];
                char** lines = rollback_to_diff(restore, &n);
                write_lines(restore->stored_path, lines, n);
                break;
            }
            // if the file was modified.
            case (E_DIFF_FILE_MODIFIED): {
                size_t index = search_commit_name(commit, diff->new_path);
                // lost in some commits via rename, made somewhere between
                // this commit and the current commit.
                if (index == -1) {
                    remove(diff->new_path);
                    index = search_commit_name(commit, diff->stored_path);
                    if (index == -1)
                        break;
                }

                // write the lines to the file.
                int n = 0;
                diff_t* restore = commit->changes[index];
                char** lines = rollback_to_diff(restore, &n);
                write_lines(restore->new_path, lines, n);
                break;
            }
            // if a folder was created
            case (E_DIFF_NEW_FOLDER): {
                // then we 'unlink' this folder.
                remove(diff->new_path);
                break;
            }
            // if a folder was deleted
            case (E_DIFF_FOLDER_DELETED): {
                size_t index = search_commit_name(commit, diff->stored_path);
                if (index == -1) continue;

                // make a new folder.
                mkdir(diff->stored_path, 0755);
                break;
            }
            default: ; //???
        }
    }
    branch->idx = idx;
};

/**
 * @brief checkout to a newer commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void checkout(branch_t* branch, const commit_t* commit) {
    // find the differences between <branch>'s commit and <commit>.
    commit_t* current = branch->commits[branch->idx];

    // first thing to do is to check that this commit is in <branch> history.
    long idx = -1;
    for (size_t i = 0; i < branch->count; i++) {
        // we compare by hashes, not by pointers.
        if (!strcmp(branch->commits[i]->hash, commit->hash)) {
            idx = i;
            break;
        }
    }

    // if the commit is not in the history, report the error and return.
    if (idx == -1) {
        fprintf(stderr,"commit not found in branch history.\n");
        exit(-1);
    }

    // iterate for a "delta apply".
    for (size_t i = 0; i < commit->count; i++) {
        diff_t* diff = commit->changes[i];
        switch (diff->type) {
            // if a file was created and is not in our commit, delete it.
            case (E_DIFF_NEW_FILE): {
                // write this out to the file.
                int n = 0;
                char** lines = rollback_to_diff(diff, &n);
                write_lines(diff->stored_path, lines, n);
                break;
            }
            // if the file was modified.
            case (E_DIFF_FILE_MODIFIED): {
                if (strcmp(diff->new_path, diff->stored_path) != 0) {
                    // if the file was renamed, we need to remove the old file.
                    remove(diff->stored_path);
                }

                // write the lines to the file.
                int n = 0;
                char** lines = rollback_to_diff(diff, &n);
                write_lines(diff->new_path, lines, n);
                break;
            }
            // if a folder was created
            case (E_DIFF_NEW_FOLDER): {
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
    branch->idx = idx;
};
