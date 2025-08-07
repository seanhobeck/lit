/**
 * @author Sean Hobeck
 * @date 2025-08-06
 *
 * @file main.c
 *    entry point for the program, we read arguments and process files
 *    and diffs from there.
 */
/*! @uses printf. */
#include <stdio.h>

/*! @uses diff_create, diff_write, diff_read. */
#include "diff.h"

/*! @uses repo_create, repository_read, repository_write. */
#include "repo.h"

/*! @uses branch_create, branch_write, branch_read, branch_add_commit. */
#include "branch.h"

/*! @uses rollback, checkout. */
#include "roll.h"

/*! @uses pvc_t*, pvc_inode_t*, pvc_collect. */
#include "pvc.h"

/*! @uses cmd_t, parse_args. */
#include "arg.h"

/*! @uses mkdir, remove */
#include <time.h>
#include <sys/stat.h>

/**
 * @brief write a diff to .lit/staging.
 *
 * @param diff pointer to the diff to be added to .lit/staging
 */
void push_diff_to_staging(const diff_t* diff) {
    // ensure the staging directory exists.
    mkdir(".lit/staging/", 0755);
    char* staging_path = malloc(512);
    snprintf(staging_path, 512, ".lit/staging/%u.diff", diff->hash);
    diff_write(diff, staging_path);
    printf("added .diff to .lit/staging\n");
    free(staging_path);
}

/** @brief entry point libc. */
int main(int argc, char **argv) {
    // parse our commandline arguments.
    cmd_t cmd;
    parse_args(argc, argv, &cmd);

    // checking each command.
    switch (cmd.type) {
        // --i or --init to initialize a new repository.
        case (E_CMD_TYPE_INIT): {
            if (repository_create() == 0x0) {
                fprintf(stderr, "failed to create repository; '.lit' directory already exists.\n");
                return -1;
            }
            printf("repository initialized successfully.\n");
            return 0;
        }
        // --s or --status to show the status of the repository.
        case (E_CMD_TYPE_STATUS):
        // --c or --commit to commit changes to the repository.
        case (E_CMD_TYPE_COMMIT): {
            // read our repository from disk.
            repository_t* repo = repository_read();

            // get the current branch.
            branch_t* current = repo->main;
            if (repo->idx != 0) {
                if (repo->idx > repo->count)
                    fprintf(stderr, "current branch index %lu is out of bounds.\n", repo->idx);
                else
                    current = repo->branches[repo->idx];
            }

            // pvc all files in .lit/staging and create a commit with the diffs.
            pvc_t* vector = pvc_collect(".lit/staging/", E_PVC_TYPE_NO_RECURSE);

            // print out our information and then return 0.
            if (cmd.type == E_CMD_TYPE_STATUS) {
                // print out the current branch name and the number of diffs staged.
                printf("current branch: '%s', %lu diffs staged, with %lu commits.\n", \
                    current->name, vector->count, current->count);
                for (size_t i = 0; i < current->count; i++) {
                    if (i == current->idx) printf("\t->  ");
                    else printf("\t    ");
                    printf("%s : %s @ %s\n", strtrm(strsha1(current->commits[i]->hash), 60), \
                        strtrm(current->commits[i]->message, 32), current->commits[i]->timestamp);
                }
                return 0;
            }

            // iterate over our path collected vector.
            commit_t* commit = commit_create(cmd.extra_cmd, current->name);
            if (vector->nodes == 0x0) {
                fprintf(stderr, "no diffs to commit; nothing staged.\n");
                remove(commit->path); /// remove commit folder.
                return -1;
            }
            size_t i = 0;
            for (; i < vector->count; i++) {
                pvc_inode_t* inode = vector->nodes[i];
                diff_t* read = diff_read(inode->path);
                commit_add_diff(commit, read);
                remove(inode->path); // remove the file from the staging directory.
                free(inode->path);
                free(inode->name);
                free(inode);
            }
            free(vector);

            // add the commit to the current branch history.
            branch_add_commit(commit, current);
            commit_write(commit);
            current->idx = current->count-1; // set the current commit index to the new commit.
            branch_write(current);
            remove(".lit/staging/"); // remove the staging directory.
            printf("added commit '%s' to branch '%s' with %lu change(s).\n", \
                commit->message, current->name, i);
            return 0;
        }
        // --C or --checkout to checkout a commit.
        case (E_CMD_TYPE_CHECKOUT):
        // --r or --rollback to rollback to a previous commit.
        case (E_CMD_TYPE_ROLLBACK): {
            // read our repository from disk.
            repository_t* repo = repository_read();

            // get the current branch.
            branch_t* current = repo->main;
            if (repo->idx != 0) {
                if (repo->idx > repo->count)
                    fprintf(stderr, "current branch index %lu is out of bounds.\n", repo->idx);
                else
                    current = repo->branches[repo->idx];
            }

            // convert user hash to a sha256_t type.
            sha1_t user_hash = {0};
            unsigned char* _hash = strtoha(cmd.extra_cmd, 20);
            memcpy(user_hash, _hash, 20);
            free(_hash);

            // rollback to the commit specified by the user,
            // this is a hash that must be specified (TODO: support names or indices via -idx <index>).
            commit_t* commit = 0x0;
            size_t i = 0;
            for (; i < current->count; i++) {
                if (!strcmp(current->commits[i]->hash, user_hash)) {
                    commit = current->commits[i];
                    break;
                }
            }
            // if the commit was not found, report the error and return.
            if (commit == 0x0) {
                fprintf(stderr, "commit-'%s' not found.\n", cmd.extra_cmd);
                return -1;
            }
            if (cmd.type == E_CMD_TYPE_ROLLBACK) {
                // if the commit is newer than the current commit, report the error and return.
                if (i >= current->idx) {
                    fprintf(stderr, "cannot rollback to a commit that is newer than the current commit.\n");
                    return -1;
                }
                // rollback to the commit by applying the diffs in reverse order.
                rollback(current, commit);
            }
            else {
                // if the commit is older than the current commit, report the error and return.
                if (i <= current->idx) {
                    fprintf(stderr, "cannot checkout to a commit that is older than the current commit.\n");
                    return -1;
                }
                // checkout to the commit by applying the diffs in order.
                checkout(current, commit);
            }

            // write and leave.
            current->idx = i;
            branch_write(current);
            break;
        }
        // --d or --delete to delete a file or folder from the branch.
        case (E_CMD_TYPE_DELETE_INODE):
        // --a or --add to add a file or folder to the branch.
        case (E_CMD_TYPE_ADD_INODE): {
            // read our repository from disk.
            repository_t* repo = repository_read();
            if (repo == 0x0) {
                perror("failed to read repository from disk.\n");
                return -1;
            }

            // if we are adding a file or folder, we need to create a diff for it.
            diff_t* diff;
            if (cmd.type == E_CMD_TYPE_ADD_INODE) {
                // create a new diff for the file or folder.
                if (cmd.extra_cmd[strlen(cmd.extra_cmd) - 1] == '/')
                    diff = diff_folder(cmd.extra_cmd, E_DIFF_NEW_FOLDER);
                else
                    diff = diff_file(cmd.extra_cmd, E_DIFF_NEW_FILE);
            }
            else {
                // create a delete diff for the file or folder.
                if (cmd.extra_cmd[strlen(cmd.extra_cmd) - 1] == '/')
                    diff = diff_folder(cmd.extra_cmd, E_DIFF_FOLDER_DELETED);
                else
                    diff = diff_file(cmd.extra_cmd, E_DIFF_FILE_DELETED);

                // delete the file.
                remove(diff->stored_path);
            }

            // error checking.
            if (diff->new_path == 0x0) {
                fprintf(stderr, "failed to create diff for '%s'.\n", cmd.extra_cmd);
                return -1;
            }

            // push to staging.
            push_diff_to_staging(diff);
            return 0;
        }
        // --m or --modified to add a modified file or folder to the branch.
        case (E_CMD_TYPE_MODIFIED_INODE): {
            // read our repository from disk.
            repository_t* repo = repository_read();

            // get the current branch.
            branch_t* current = repo->main;
            if (repo->idx != 0) {
                if (repo->idx > repo->count)
                    fprintf(stderr, "current branch index %lu is out of bounds.\n", repo->idx);
                else
                    current = repo->branches[repo->idx];
            }

            // what filename are we looking for?
            if (cmd.extra_cmd[strlen(cmd.extra_cmd) - 1] == '/') {
                // if this is a folder that we are dealing with then we need
                //  to create a new diff for the folder, and then a delete diff
                //  for the folder.
                if (cmd.extra_data != 0x0 && cmd.extra_data[strlen(cmd.extra_data) - 1] == '/') {
                    /// create a diff to delete the old folder, and then create a new one, but first we have to check that the older one exists.
                    diff_t* new_folder = diff_folder(cmd.extra_data, E_DIFF_NEW_FOLDER);
                    diff_t* old_folder = diff_folder(cmd.extra_cmd, E_DIFF_FOLDER_DELETED);

                    // check if the folders exist.
                    struct stat osb;
                    if (stat(old_folder->stored_path, &osb) != 0) {
                        perror("stat failed; old folder not found.\n");
                        return -1;
                    }

                    // ensure the staging directory exists
                    mkdir(".lit/staging/", 0755);

                    // stage both diffs
                    char staging_path[512];
                    snprintf(staging_path, sizeof(staging_path), ".lit/staging/%u.diff", old_folder->hash);
                    diff_write(old_folder, staging_path);
                    snprintf(staging_path, sizeof(staging_path), ".lit/staging/%u.diff", new_folder->hash);
                    diff_write(new_folder, staging_path);

                    // print and rename the folder.
                    printf("added folder modification diff for '%s' -> '%s' to .lit/staging\n",
                           cmd.extra_cmd, cmd.extra_data);
                    rename(cmd.extra_cmd, cmd.extra_data);
                    return 0;
                }

                // print a error.
                perror("cannot modify a folder to be a file, or write a .diff for a folder that hasn't been renamed.\n");
                return -1;
            }

            // iterate to find the most recent commit on the current branch.
            size_t index = current->idx;
            diff_t* recent_diff = 0x0;

            // we need to make the old file under a temp name, pass it through to
            //  the diff_file_modified, and then create a diff for it and stage it.
            for (size_t i = index - 1; i != (size_t) -1; i--) {
                commit_t* commit = current->commits[i];

                // search for the file in the commit using its new_path first.
                for (size_t j = 0; j < commit->count; j++) {
                    diff_t* diff = commit->changes[j];

                    // if the new_path on a diff is the original file we are looking for, then we need to re-construct it.
                    if (diff->new_path && !strcmp(diff->new_path, cmd.extra_cmd)) {
                        if (diff->type == E_DIFF_FILE_MODIFIED || diff->type == E_DIFF_NEW_FILE) {
                            recent_diff = diff;
                            goto found;
                        }
                    }
                }
            }
            perror("file not found in previous commits on this branch.\n");
            return -1;
            found:

            // create a temporary file with the original content
            char temp_path[512];
            snprintf(temp_path, sizeof(temp_path), ".lit/%ld.tmp", time(0x0));

            // reconstruct the original file from the diff
            int line_count = 0;
            char** original_lines = rollback_to_diff(recent_diff, &line_count);

            if (!original_lines) {
                fprintf(stderr, "error: failed to reconstruct original file content.\n");
                return -1;
            }

            // write original content to temp file
            FILE* f = fopen(temp_path, "w");
            if (!f) {
                perror("fopen failed; could not open temp file for writing.\n");
                return -1;
            }
            for (size_t i = 0; i < line_count; i++) {
                fprintf(f, "%s\n", original_lines[i]);
            }
            fclose(f);

            // determine the new path (use extra_data if provided for renaming)
            const char* new_path = cmd.extra_data ? cmd.extra_data : cmd.extra_cmd;

            // create the modified diff between original and current
            diff_t* diff = diff_file_modified(temp_path, new_path);
            diff->stored_path = strdup(cmd.extra_cmd); // set the stored path to the original file (for rollback purposes)

            // clean up temp file and allocated lines
            remove(temp_path);
            for (int k = 0; k < line_count; k++) {
                if (original_lines[k]) free(original_lines[k]);
            }
            free(original_lines);

            push_diff_to_staging(diff);
            return 0;
        }
        // --h and --v have already been handled in <parse_args>.
        case (E_CMD_TYPE_HELP):
        case (E_CMD_TYPE_VERSION):
        default: {};
    }

    return 0;
};