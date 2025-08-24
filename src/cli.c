/**
 * @author Sean Hobeck
 * @date 2025-08-15
 *
 * @file cli.h
 *    the cli module of lit, responsible for handling the arguments passed by the user and
 *    doing certain actions (CRUD, repository actions, etc).
 */
#include "cli.h"

/*! @uses printf. */
#include <stdio.h>

/*! @uses diff_create, diff_write, diff_read. */
#include "diff.h"

/*! @uses repo_create, repository_read, repository_write. */
#include "repo.h"

/*! @uses branch_create, branch_write, branch_read, branch_add_commit. */
#include "branch.h"

/*! @uses rebase_branch, e_rebase_result_t. */
#include "rebase.h"

/*! @uses rollback, checkout. */
#include "ops.h"

/*! @uses pvc_t*, pvc_inode_t*, pvc_collect. */
#include "pvc.h"

/*! @uses mkdir, remove */
#include <sys/stat.h>

/*! @uses time_t */
#include <time.h>

/*! @uses scan_object_cache */
#include "cache.h"

/*! @uses tag_t, create_tag, write_tag, ... */
#include "tag.h"

/*! @uses bool, true, false */
#include <stdbool.h>


/**
 * @brief write a diff to .lit/stashed.
 *
 * @param diff pointer to the diff to be added to .lit/stashed
 */
void
stash_change(const diff_t* diff) {
    // ensure the staging/stashed directory exists.
    mkdir(".lit/stashed/", 0755);
    char* staging_path = malloc(512);
    snprintf(staging_path, 512, ".lit/stashed/%u", diff->crc);
    write_diff(diff, staging_path);
    printf("added .diff to .lit/stashed\n");
    free(staging_path);
}

/**
 * @brief handle the arguments passed by the user as a cli (command-line interface) tool.
 *
 * @param args the arg_t structure created when parsing argc & argv.
 * @return the return code if needed.
 */
int
cli_handle(arg_t args) {
    // checking the argument structure.
    switch (args.type) {
        // -i or --init to initialize a new repository.
        case E_ARG_TYPE_INIT: {
            if (create_repository() == 0x0) {
                fprintf(stderr, "failed to create repository; '.lit' directory already exists.\n");
                return -1;
            }
            printf("repository initialized successfully.\n");
            return 0;
        }
        // -s or --status to show the status of the repository.
        case E_ARG_TYPE_STATUS:
        // -c or --commit to commit changes to the repository.
        case E_ARG_TYPE_COMMIT: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // get the current branch.
            branch_t* current = repo->branches[repo->idx];

            // pvc all files in .lit/staging and create a commit with the diffs.
            vector_t* vector = vector_collect(".lit/stashed/", E_PVC_TYPE_NO_RECURSE);

            // print out our information and then return 0.
            if (args.type == E_ARG_TYPE_STATUS) {
                // print out the current branch name and the number of diffs stashed.
                printf("current branch: \'%s\', %lu change(s) stashed, with %lu commit(s).\n", \
                    current->name, vector->count, current->count);

                // iterate through each object.
                for (size_t i = 0; i < current->count; i++) {
                    if (i == current->idx) printf("\t->  ");
                    else printf("\t    ");

                    // print the information about the commits.
                    char* _hash = strsha1(current->commits[i]->hash);
                    printf("%s : %s @ %s\n", strtrm(_hash, 60), \
                        strtrm(current->commits[i]->message, 32), current->commits[i]->timestamp);
                    free(_hash);
                }

                // print out all of the tags (on this branch).
                tag_list_t* tags = read_tags();
                tags = filter_tags(current->hash, tags);
                if (tags->count >= 0) {
                    printf("\ntags:\n");
                    for (size_t i = 0; i < tags->count; i++) {
                        printf("\t    %s -> %s\n", tags->tags[i]->name, \
                            strsha1(tags->tags[i]->commit_hash));
                    }
                }
                return 0;
            }

            // iterate over our path collected vector.
            commit_t* commit = create_commit(args.argv[2], current->name);
            if (vector->nodes == 0x0) {
                fprintf(stderr, "no diffs to commit; nothing stashed.\n");
                remove(commit->path); /// remove commit folder.
                return -1;
            }
            size_t i = 0;
            for (; i < vector->count; i++) {
                vinode_t* inode = vector->nodes[i];
                diff_t* read = read_diff(inode->path);
                add_diff_commit(commit, read);
                remove(inode->path); // remove the file from the staging directory.
                free(inode->path);
                free(inode->name);
                free(inode);
            }
            free(vector);

            // add the commit to the current branch history.
            add_commit_branch(commit, current);
            write_commit(commit);
            current->idx = current->count-1; // set the current commit index to the new commit.
            write_branch(current);
            remove(".lit/stashed/"); // remove the staging directory.
            printf("added commit '%s' to branch '%s' with %lu change(s).\n", \
                strtrm(commit->message, 32), current->name, i);
            return 0;
        }
        // -C or --checkout to checkout a commit.
        case E_ARG_TYPE_CHECKOUT:
        // -r or --rollback to rollback to a previous commit.
        case E_ARG_TYPE_ROLLBACK: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // get the current branch.
            branch_t* current = repo->branches[repo->idx];

            // convert user hash to a sha1_t type.
            sha1_t user_hash = {0};
            unsigned char* _hash = strtoha(args.argv[2], 20);
            memcpy(user_hash, _hash, 20);
            free(_hash);

            // rollback to the commit specified by the user,
            // this is a hash that must be specified (TODO: support tags?).
            commit_t* commit = 0x0;
            size_t i = 0;
            for (; i < current->count; i++) {
                if (!strncmp(current->commits[i]->hash, user_hash, 20)) {
                    commit = current->commits[i];
                    break;
                }
            }
            // if the commit was not found, report the error and return.
            if (commit == 0x0) {
                fprintf(stderr, "commit '%s' not found.\n", args.argv[2]);
                return -1;
            }
            if (args.type == E_ARG_TYPE_ROLLBACK) {
                // if the commit is newer than the current commit, report the error and return.
                if (i >= current->idx) {
                    fprintf(stderr, "cannot rollback to a commit that is newer than the current commit.\n");
                    return -1;
                }
                // rollback to the commit by applying the diffs in reverse order.
                rollback(current, commit);
                printf("rolled back to \'%s\' on branch \'%s\'\n", \
                    strtrm(strsha1(commit->hash), 12), current->name);
            }
            else {
                // if the commit is older than the current commit, report the error and return.
                if (i <= current->idx) {
                    fprintf(stderr, "cannot checkout to a commit that is older than the current commit.\n");
                    return -1;
                }
                // checkout to the commit by applying the diffs in order.
                checkout(current, commit);
                printf("checked out \'%s\' on branch \'%s\'\n", \
                    strtrm(strsha1(commit->hash), 12), current->name);
            }

            // write and leave.
            current->idx = i;
            write_branch(current);

            // write out an warning (can be disabled).
            printf("\e[0;33mwarning, treat rollbacks and checkouts as readonly.\n"
                   "changing any files could damage your control tree.\n\e[0m");
            break;
        }
        // -d or --delete to delete a file or folder from the branch.
        case E_ARG_TYPE_DELETE_INODE:
        // -a or --add to add a file or folder to the branch.
        case E_ARG_TYPE_ADD_INODE: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // if we are adding a file or folder, we need to create a diff for it.
            diff_t* diff;
            if (args.type == E_ARG_TYPE_ADD_INODE) {
                // create a new diff for the file or folder.
                if (args.argv[2][strlen(args.argv[2]) - 1] == '/')
                    diff = create_folder_diff(args.argv[2], E_DIFF_FOLDER_NEW);
                else
                    diff = create_file_diff(args.argv[2], E_DIFF_FILE_NEW);
            }
            else {
                // create a delete diff for the file or folder.
                if (args.argv[2][strlen(args.argv[2]) - 1] == '/')
                    diff = create_folder_diff(args.argv[2], E_DIFF_FOLDER_DELETED);
                else
                    diff = create_file_diff(args.argv[2], E_DIFF_FILE_DELETED);

                // delete the file.
                remove(diff->stored_path);
            }

            // error checking.
            if (diff->new_path == 0x0) {
                fprintf(stderr, "failed to create diff for '%s'.\n", args.argv[2]);
                return -1;
            }

            // push to staging.
            stash_change(diff);
            return 0;
        }
        // -m or --modified to add a modified file or folder to the branch.
        case E_ARG_TYPE_MODIFIED_INODE: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // get the current branch.
            branch_t* current = repo->branches[repo->idx];

            // what filename are we looking for?
            if (args.argv[2][strlen(args.argv[2]) - 1] == '/') {
                // if this is a folder that we are dealing with then we need
                //  to create a new diff for the folder, and then a delete diff
                //  for the folder.
                if (args.argv[3] != 0x0 && args.argv[3][strlen(args.argv[3]) - 1] == '/') {
                    /// create a diff to delete the old folder, and then create a new one, but first we have to check that the older one exists.
                    diff_t* new_folder = create_folder_diff(args.argv[3], E_DIFF_FOLDER_NEW);
                    diff_t* old_folder = create_folder_diff(args.argv[2], E_DIFF_FOLDER_DELETED);

                    // check if the folders exist.
                    struct stat osb;
                    if (stat(old_folder->stored_path, &osb) != 0) {
                        fprintf(stderr,"stat failed; old folder not found.\n");
                        return -1;
                    }

                    // write the changes to stashed.
                    stash_change(new_folder);
                    stash_change(old_folder);

                    // print and rename the folder.
                    printf("added changes for '%s' -> '%s' to stashed\n",
                           args.argv[2], args.argv[3]);
                    rename(args.argv[2], args.argv[3]);
                    return 0;
                }

                // print a error.
                fprintf(stderr,"cannot modify a folder to be a file, or write a .diff for a folder that hasn't been renamed.\n");
                return -1;
            }

            // iterate to find the most recent commit on the current branch.
            size_t index = current->idx;
            diff_t* recent_diff = 0x0;

            // we need to make the old file under a temp name, pass it through to
            //  the diff_file_modified, and then create a diff for it and stage it.
            for (size_t i = index; i != (size_t) -1; i--) {
                commit_t* commit = current->commits[i];

                // search for the file in the commit using its new_path first.
                for (size_t j = 0; j < commit->count; j++) {
                    diff_t* diff = commit->changes[j];

                    // if the new_path on a diff is the original file we are looking for, then we need to re-construct it.
                    if (diff->new_path && !strcmp(diff->new_path, args.argv[2])) {
                        if (diff->type == E_DIFF_FILE_MODIFIED || diff->type == E_DIFF_FILE_NEW) {
                            recent_diff = diff;
                            goto found;
                        }
                    }
                }
            }
            fprintf(stderr,"file not found in previous commits on this branch.\n");
            return -1;
            found:

            // create a temporary file with the original content
            char temp_path[512];
            snprintf(temp_path, sizeof(temp_path), ".lit/%ld.tmp", time(0x0));

            // reconstruct the original file from the diff
            size_t line_count = 0;
            char** original_lines = fcleanls(recent_diff->lines, recent_diff->count, &line_count);
            if (!original_lines) {
                fprintf(stderr, "error: failed to reconstruct original file content.\n");
                return -1;
            }

            // write original content to temp file
            FILE* f = fopen(temp_path, "w");
            if (!f) {
                fprintf(stderr,"fopen failed; could not open temp file for writing.\n");
                return -1;
            }
            for (size_t i = 0; i < line_count; i++) {
                fprintf(f, "%s\n", original_lines[i]);
            }
            fclose(f);

            // determine the new path (use extra data if provided for renaming)
            const char* new_path = args.argv[3] ? args.argv[3] : args.argv[2];

            // create the modified diff between original and current
            diff_t* diff = create_file_modified_diff(temp_path, new_path);
            diff->stored_path = strdup(args.argv[2]); // set the stored path to the original file (for rollback purposes)

            // clean up temp file and allocated lines
            remove(temp_path);
            stash_change(diff);
            return 0;
        }
        // -aB or --add-branch to create a new branch of main's head.
        case E_ARG_TYPE_CREATE_BRANCH: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // get the current branch.
            branch_t* current = repo->branches[repo->idx];
            create_branch_repository(repo, args.argv[2]);
            printf("created branch '%s' from '%s'.\n", args.argv[2], current->name);
            break;
        }
        // -dB or --delete-branch to delete a branch from the repository.
        case E_ARG_TYPE_DELETE_BRANCH: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // get the current branch.
            branch_t* current = repo->branches[repo->idx];
            delete_branch_repository(repo, args.argv[2]);

            // switching back to the main branch.
            printf("deleted branch '%s'.\n", args.argv[2]);
            if (!strcmp(current->name, args.argv[2])) {
                printf("switching back to main branch.\n");
                switch_branch_repository(repo, "origin");
            }
            write_repository(repo);
            break;
        };
        // -sB or --switch-branch to switch to a different branch in the repository.
        case E_ARG_TYPE_SWITCH_BRANCH: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // switching...
            switch_branch_repository(repo, args.argv[2]);
            printf("switched to branch '%s'.\n", args.argv[2]);
            break;
        };
        // -rB or --rebase-branch to rebase a branch onto anothers head.
        case E_ARG_TYPE_REBASE_BRANCH: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // rebase onto the branch provided.
            return rebase_branch(repo, args.argv[2], args.argv[3]) \
                == E_REBASE_RESULT_SUCCESS ? 0 : 1;
        }
        // -cc or --clear-cache to clear any caches leftover from merges and or rebases.
        case E_ARG_TYPE_CLEAR_CACHE: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // scan the object cache...
            return scan_object_cache(repo) == E_CACHE_RESULT_SUCCESS ? 0 : 1;
        }
        // -aT or --add-tag to add a tag to a commit
        case E_ARG_TYPE_ADD_TAG: {
            // read our repository from disk.
            repository_t* repo = read_repository();

            // convert the hash to sha1_t type.
            unsigned char* _hash = strtoha(args.argv[2], 20);

            // find the commit for this tag, (if it exists)
            commit_t* commit = 0x0;
            branch_t* active = repo->branches[repo->idx];
            for (size_t i = 0; i < active->count; i++) {

                // memcmp the sha1 hashes.
                if (memcmp(_hash, active->commits[i]->hash, 20) == 0) {
                    commit = active->commits[i];
                    break;
                }
            }

            // if we did not find the commit hash in the branches history
            //  report it back to the user.
            if (!commit) {
                fprintf(stderr, "did not find commit hash \'%s\' in active branches history.\n", args.argv[2]);
                return -1;
            }

            // create the tag given the hash and the
            tag_t* tag = create_tag(active, commit, args.argv[3]);
            free(_hash);

            // then write the tag.
            write_tag(tag);
            return 0;
        }
        // -dT or --delete-tag to delete a tag from the branches history.
        case E_ARG_TYPE_DELETE_TAG: {
            // call read_tag on the file location, and if
            //  it is correct, then we can remove it.
            tag_list_t* tags = read_tags();
            bool found = false;
            for (size_t i = 0; i < tags->count; i++) {
                if (!strcmp(tags->tags[i]->name, args.argv[2])) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "tag \'%s\' not found.\n", args.argv[2]);
                return -1;
            }

            // create the temp path for the tag in '.lit/refs/tags/<name>',
            // and then remove it.
            char path[256];
            snprintf(path, 256, ".lit/refs/tags/%s", args.argv[2]);
            remove(path);
            return 0;
        }

        // --h and --v have already been handled in <parse_args>.
        case E_ARG_TYPE_HELP:
        case E_ARG_TYPE_VERSION:
        default: {};
    }
    return 0;
};