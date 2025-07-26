/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file roll.c
 *    the branch module in the version control system, it is responsible for handling
 *    commits and locating diffs and changes between branches.
 */
#include <roll.h>

/*! @uses fopen, FILE*, fclose, fprintf */
#include <stdio.h>

/*! @uses malloc, free */
#include <stdlib.h>

/*! @uses mkdir */
#include <sys/stat.h>

/*! @uses strcmp */
#include <string.h>

/**
 * @brief write rollback lines to a file path provided.
 *
 * @param path the path to the file to be created/written to.
 * @param lines the allocated number of lines from rollback_diff.
 * @param n the number of lines in the rolled-back diff.
 */
void write_lines(const char* path, char** lines, int n) {
    // open the file.
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("fopen failed; could not open file for rollback writing.\n");
        return;
    }

    // iterate over the lines and use fprintf();
    for (unsigned int i = 0u; i < n; i++) {
        fprintf(f, "%s\n", lines[i]);
    }
    fclose(f);
};

/**
 * @brief search through a commit to find if a diff is in it.
 *
 * @param commit the commit to search in.
 * @param diff the diff to be searched for.
 * @return if <diff> was found in <commit> (-1 -> no, > 0 yes).
 */
int search_commit_diff(const commit_t* commit, const diff_t* diff) {
    // iterate over each diff in the commit.
    for (unsigned long i = 0u; i < commit->n_diffs; i++)
        if (commit->diffs[i] == diff)
            return i;
    return -1;
}

/**
 * @brief search through a commit to find if a name is in any of the diffs.
 *
 * @param commit the commit to search in.
 * @param name the name to be searched for.
 * @return if <name> was found in <commit>'s diffs (-1 -> no, > 0 yes).
 */
int search_commit_name(const commit_t* commit, const char* name) {
    // iterate over each diff in the commit.
    for (unsigned long i = 0u; i < commit->n_diffs; i++)
        if (!strcmp(commit->diffs[i]->n_name, name))
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
    char** lines = malloc(diff->count * sizeof(char*));

    // iterate...
    int m = 0;
    for (unsigned int i = 0; i < diff->count; i++) {
        char* line = diff->lines[i];

        // if there is a space, there isn't a two char prefix.
        if (line[0] == ' ') {
            lines[m++] = strdup(line+1u);
        }
        else if (line[0] == '+') {
            // skip the two characters '+ '
            lines[m++] = strdup(line+2u);
        }
    }
    *n = m;
    return lines;
};

/**
 * @brief rollback / checkout a older commit.
 *
 * @param branch the current branch that we are on.
 * @param commit the selected commit to be rolled back to.
 */
void rollback(branch_t* branch, const commit_t* commit) {
    // find the differences between <branch>'s commit and <commit>.
    commit_t* current = branch->history.commits[branch->current_commit];

    // first thing to do is to check that this commit is in <branch> history.
    long idx = -1;
    for (unsigned long i = 0u; i < branch->history.n_commits; i++) {
        if (branch->history.commits[i] == commit) {
            idx = i;
            break;
        }
    }

    // if the commit is not in the history, report the error and return.
    if (idx == -1) {
        perror("commit not found in branch history.\n");
        return;
    }

    // iterate for a "delta apply".
    for (unsigned long i = 0u; i < current->n_diffs; i++) {
        diff_t* diff = current->diffs[i];
        switch (diff->type) {
            // if a file was created and is not in our commit, delete it.
            case (E_DIFF_NEW_FILE): {
                // then we 'unlink' this diff.
                if (search_commit_name(commit, diff->n_name) == -1)
                    remove(diff->n_name);
                break;
            }
            // if a file was deleted and is in our commit, restore it.
            case (E_DIFF_FILE_DELETED): {
                int index = search_commit_name(commit, diff->s_name);
                if (index == -1) continue;

                // write this out to the file.
                int n = 0;
                diff_t* restore = commit->diffs[index];
                char** lines = rollback_to_diff(restore, &n);
                write_lines(restore->n_name, lines, n);
                break;
            }
            // if the file was modified.
            case (E_DIFF_FILE_MODIFIED): {
                int index = search_commit_name(commit, diff->n_name);
                // lost in some commits via rename, made somewhere between
                // this commit and the current commit.
                if (index == -1) {
                    remove(diff->n_name);
                    index = search_commit_name(commit, diff->s_name);
                    if (index == -1)
                        break;
                }

                // write the lines to the file.
                int n = 0;
                diff_t* restore = commit->diffs[index];
                char** lines = rollback_to_diff(restore, &n);
                write_lines(restore->n_name, lines, n);
                break;
            }
            // if a folder was created
            case (E_DIFF_NEW_FOLDER): {
                // then we 'unlink' this folder.
                if (search_commit_name(commit, diff->n_name) == -1)
                    remove(diff->n_name);
                break;
            }
            // if a folder was deleted
            case (E_DIFF_FOLDER_DELETED): {
                int index = search_commit_name(commit, diff->s_name);
                if (index == -1) continue;

                // make a new folder.
                mkdir(diff->s_name, 0777);
                break;
            }
        }
    }
    branch->current_commit = idx;
};