/**
 * @author Sean Hobeck
 * @date 2025-08-29
 *
 * @file cli.h
 *    the cli module of lit, responsible for handling the arguments passed by the user and
 *    doing certain actions (diff, commit, branch, repository).
 */
#include "cli.h"

/*! @uses printf. */
#include <stdio.h>

/*! @uses mkdir, remove */
#include <sys/stat.h>

/*! @uses time_t */
#include <time.h>

/*! @uses bool, true, false */
#include <stdbool.h>

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

/*! @uses scan_object_cache */
#include "cache.h"

/*! @uses tag_t, create_tag, write_tag, ... */
#include "tag.h"

/*! @uses shelve_changes */
#include "shelve.h"

/*! @uses config_t, read_config */
#include "conf.h"

/// static variables.
repository_t* repository;
branch_t* branch;
config_t* config;

/**
 * @brief setup the handlers to set <repository> and <branch> to the
 *  current repository and branch.
 */
void
setup() {
    // read our repository from disk.
    repository = read_repository();
    assert(repository != 0x0);

    // get the active branch.
    branch = repository->branches[repository->idx];
    assert(branch != 0x0);

    // read the config file.
    config = read_config();
};

/**
 * @brief handle the init operation, creating a new repository in the
 *  current working directory.
 *
 * @return the return code (0 if not required).
 */
int
handle_init() {
    // @ref[repo.c:113]
    if (create_repository() == 0x0) {
        fprintf(stderr, "repository already exists.\n");
        return -1;
    }
    printf("repository initialized successfully.\n");
    return 0;
};

/**
 * @brief handle the status operation, print out the information about the
 *  current repository and branch.
 *
 * @return the return code (0 if not required).
 */
int
handle_status() {
    // pvc all files in .lit/objects/shelved/<branch_name> and
    //  create a commit with the diffs.
    vector_t* vector = shelve_collect(branch->name);

    // print out the active branch name and the number of diffs shelved.
    printf("current branch: \'%s\', %lu change(s) shelved, with %lu commit(s), %s.\n", \
        branch->name, vector->count, branch->count,
        repository->readonly ? "in read-only " : "in read-write");

    // iterate through each object.
    for (size_t i = 0; i < branch->count; i++) {
        if (i == branch->idx) printf("\t->  ");
        else printf("\t    ");

        // print the information about the commits.
        char* _hash = strsha1(branch->commits[i]->hash);
        printf("%s : %s @ %s\n", strtrm(_hash, 60), \
            strtrm(branch->commits[i]->message, 32), branch->commits[i]->timestamp);
        free(_hash);
    }

    // print out all the tags (on this branch).
    tag_list_t* tags = read_tags();
    tags = filter_tags(branch->hash, tags);
    if (tags->count > 0) {
        printf("\ntag(s):\n");
        for (size_t i = 0; i < tags->count; i++) {
            printf("\t    %s -> %s\n", tags->tags[i]->name, \
                strsha1(tags->tags[i]->commit_hash));
        }
    }
    return 0;
};

/**
 * @brief handle the commit operation, creating a new commit from the staged
 *  changes.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_commit(arg_t args) {
    // if we are in read-only mode we cannot commit or make changes.
    if (repository->readonly) {
        fprintf(stderr, "cannot commit changes in read-only mode.\n");
        return -1;
    };

    // pvc all files in .lit/objects/shelved/<branch_name> and
    //  create a commit with the diffs.
    vector_t* vector = shelve_collect(branch->name);

    // create the commit.
    commit_t* commit = create_commit(args.argv[2], branch->name);
    if (vector->nodes == 0x0) {
        fprintf(stderr, "no diffs to commit; nothing stashed.\n");
        remove(commit->path);
        return -1;
    }

    // iterate through each pvc_inode_t in the vector and read the diff from disk,
    size_t i = 0;
    for (; i < vector->count; i++) {
        vinode_t* inode = vector->nodes[i];
        diff_t* read = read_diff(inode->path);
        add_diff_commit(commit, read);
        remove(inode->path);
    }
    vector_free(vector);

    // add the commit to the active branch history.
    add_commit_branch(commit, branch);
    write_commit(commit);
    branch->idx = branch->count-1; // set the active commit index to the new commit.
    write_branch(branch);

    // remove the staging directory.
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", branch->name);
    remove(path);

    // log out to the console.
    printf("added commit '%s' to branch '%s' with %lu change(s).\n", \
        strtrm(commit->message, 32), branch->name, i);
    return 0;
};

/**
 * @brief handle the checkout or rollback operation, moving the active commit
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_cr_move(arg_t args) {
    // convert user hash to a sha1_t type.
    sha1_t user_hash = {0};
    unsigned char* _hash = strtoha(args.argv[2], 20);
    memcpy(user_hash, _hash, 20);
    free(_hash);

    // rollback to the commit specified by the user,
    // this is a hash that must be specified (TODO: support tags?).
    commit_t* commit = 0x0;
    size_t i = 0;
    for (; i < branch->count; i++) {
        if (!strncmp(branch->commits[i]->hash, user_hash, 20)) {
            commit = branch->commits[i];
            break;
        }
    }
    // if the commit was not found, report the error and return.
    if (commit == 0x0) {
        fprintf(stderr, "commit '%s' not found.\n", args.argv[2]);
        return -1;
    }
    if (args.type == E_ARG_TYPE_ROLLBACK) {
        // if the commit is newer than the active commit, report the error and return.
        if (i >= branch->idx) {
            fprintf(stderr, "cannot rollback to a commit that is newer than the active commit.\n");
            return -1;
        }
        // rollback to the commit by applying the diffs in reverse order.
        rollback(branch, commit);
        printf("rolled back to \'%s\' on branch \'%s\'\n", \
            strtrm(strsha1(commit->hash), 12), branch->name);
    }
    else {
        // if the commit is older than the active commit, report the error and return.
        if (i <= branch->idx) {
            fprintf(stderr, "cannot checkout to a commit that is older than the active commit.\n");
            return -1;
        }
        // checkout to the commit by applying the diffs in order.
        checkout(branch, commit);
        printf("checked out \'%s\' on branch \'%s\'\n", \
            strtrm(strsha1(commit->hash), 12), branch->name);
    }

    // write and leave.
    branch->idx = i;
    write_branch(branch);

    // if we are not on the latest commit, set the repository to read-only.
    repository->readonly = (i != branch->count - 1);
    write_repository(repository);

    // write out an warning
    if (!config->debug) {
        printf("\e[0;33mwarning, treat rollbacks and checkouts as readonly.\n"
               "changing any files could damage your control tree.\n\e[0m");
    }
    return 0;
};

/**
 * @brief handle the add or delete inode operation, adding or deleting a
 *  file or folder
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_add_delete_inode(arg_t args) {
    // if we are in read-only mode we cannot commit or make changes.
    if (repository->readonly) {
        fprintf(stderr, "cannot commit changes in read-only mode.\n");
        return -1;
    };

    // if we are adding a file or folder, we need to create a diff for it.
    diff_t* diff;

    // create a new diff for the file or folder.
    if (args.argv[2][strlen(args.argv[2]) - 1] == '/') {
        diff = create_folder_diff(args.argv[2],
            args.type == E_ARG_TYPE_ADD_INODE ?
            E_DIFF_FOLDER_NEW : E_DIFF_FOLDER_DELETED);
    }
    else {
        diff = create_file_diff(args.argv[2],
            args.type == E_ARG_TYPE_ADD_INODE ?
            E_DIFF_FILE_NEW : E_DIFF_FILE_DELETED);
    }

    // if we are deleting a folder, we need to remove it.
    if (args.type == E_ARG_TYPE_DELETE_INODE) {
        remove(diff->stored_path);
    }

    // error checking.
    if (diff->new_path == 0x0) {
        fprintf(stderr, "failed to create diff for '%s'.\n", args.argv[2]);
        return -1;
    }

    // push to staging for the active branch.
    shelve_changes(branch->name, diff);
    return 0;
};

/**
 * @brief handle the modified inode operation, modifying or renaming a file and
 *  or folder.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_modified_inode(arg_t args) {
    // if we are in read-only mode we cannot commit or make changes.
    if (repository->readonly) {
        fprintf(stderr, "cannot commit changes in read-only mode.\n");
        return -1;
    };

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
            shelve_changes(branch->name, new_folder);
            shelve_changes(branch->name, old_folder);

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

    // iterate to find the most recent commit on the active branch.
    size_t index = branch->idx;
    diff_t* recent_diff = 0x0;

    // we need to make the old file under a temp name, pass it through to
    //  the diff_file_modified, and then create a diff for it and stage it.
    for (size_t i = index; i != (size_t) -1; i--) {
        commit_t* commit = branch->commits[i];

        // search for the file in the commit using its new_path first.
        for (size_t j = 0; j < commit->count; j++) {
            diff_t* diff = commit->changes[j];

            // if the new_path on a diff is the original file we are looking for, then we need to re-construct it.
            if (diff->new_path && !strcmp(diff->new_path, args.argv[2])) {
                if (diff->type == E_DIFF_FILE_MODIFIED || diff->type == E_DIFF_FILE_NEW) {
                    recent_diff = diff;
                    break;
                }
            }
        }
    }
    if (!recent_diff) {
        fprintf(stderr,"file not found in previous commits on this branch.\n");
        return -1;
    }

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

    // create the modified diff between original and active
    diff_t* diff = create_file_modified_diff(temp_path, new_path);
    diff->stored_path = strdup(args.argv[2]); // set the stored path to the original file (for rollback purposes)

    // clean up temp file and allocated lines
    remove(temp_path);
    shelve_changes(branch->name, diff);
    return 0;
};

/**
 * @brief handle the create branch operation, creating a new branch from the
 *  current/active branches HEAD commit.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_create_branch(arg_t args) {
    create_branch_repository(repository, args.argv[2]);
    printf("created branch '%s' from '%s'.\n", args.argv[2], branch->name);
    return 0;
};

/**
 * @brief handle the delete branch operation, deleting a branch from the
 *  repository.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_delete_branch(arg_t args) {
    delete_branch_repository(repository, args.argv[2]);

    // switching back to the main branch.
    printf("deleted branch '%s'.\n", args.argv[2]);
    if (!strcmp(branch->name, args.argv[2])) {
        printf("switching back to main branch.\n");
        switch_branch_repository(repository, "origin");
    }
    write_repository(repository);
    return 0;
};

/**
 * @brief handle the switch branch operation, switching to a different branch
 *  on the repository.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_switch_branch(arg_t args) {
    // switching...
    switch_branch_repository(repository, args.argv[2]);
    printf("switched to branch '%s'.\n", args.argv[2]);
    return 0;
};

/**
 * @brief handle the rebase branch operation, rebasing one branch onto another.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_rebase_branch(arg_t args) {
    // rebase onto the branch provided.
    return rebase_branch(repository, args.argv[2],\
        args.argv[3]) == E_REBASE_RESULT_SUCCESS ? 0 : 1;
};

/**
 * @brief handle the clear cache operation, clearing leftover caches from
 *  branch operations (merges, rebases, etc).
 *
 * @return the return code (0 if not required).
 */
int
handle_clear_cache() {
    // scan the object cache...
    return scan_object_cache(repository) == E_CACHE_RESULT_SUCCESS ? 0 : 1;
}

/**
 * @brief handle the add tag operation, adding a tag to a commit.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_add_tag(arg_t args) {
    // convert the hash to sha1_t type.
    unsigned char* _hash = strtoha(args.argv[2], 20);

    // find the commit for this tag, (if it exists)
    commit_t* commit = 0x0;
    for (size_t i = 0; i < branch->count; i++) {

        // memcmp the sha1 hashes.
        if (memcmp(_hash, branch->commits[i]->hash, 20) == 0) {
            commit = branch->commits[i];
            break;
        }
    }

    // if we did not find the commit hash in the branches history
    //  report it back to the user.
    if (!commit) {
        fprintf(stderr, "did not find commit hash \'%s\' in active "
                        "branches history.\n", args.argv[2]);
        return -1;
    }

    // create the tag given the hash and the
    tag_t* tag = create_tag(branch, commit, args.argv[3]);
    free(_hash);

    // then write the tag.
    write_tag(tag);
    return 0;
}

/**
 * @brief handle the delete tag operation, deleting a tag from the repository
 *  history.
 *
 * @param args arguments passed by libc.
 * @return the return code (0 if not required).
 */
int
handle_delete_tag(arg_t args) {
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
            return handle_init();
        }
        // -s or --status to show the status of the repository.
        case E_ARG_TYPE_STATUS: {
            setup();
            return handle_status();
        };
        // -c or --commit to commit changes to the repository.
        case E_ARG_TYPE_COMMIT: {
            setup();
            return handle_commit(args);
        }
        // -C or --checkout to checkout a commit.
        case E_ARG_TYPE_CHECKOUT:
        // -r or --rollback to rollback to a previous commit.
        case E_ARG_TYPE_ROLLBACK: {
            setup();
            return handle_cr_move(args);
        }
        // -d or --delete to delete a file or folder from the branch.
        case E_ARG_TYPE_DELETE_INODE:
        // -a or --add to add a file or folder to the branch.
        case E_ARG_TYPE_ADD_INODE: {
            setup();
            return handle_add_delete_inode(args);
        }
        // -m or --modified to add a modified file or folder to the branch.
        case E_ARG_TYPE_MODIFIED_INODE: {
            setup();
            return handle_modified_inode(args);
        }
        // -aB or --add-branch to create a new branch of main's head.
        case E_ARG_TYPE_CREATE_BRANCH: {
            setup();
            return handle_create_branch(args);
        }
        // -dB or --delete-branch to delete a branch from the repository.
        case E_ARG_TYPE_DELETE_BRANCH: {
            setup();
            return handle_delete_branch(args);
        };
        // -sB or --switch-branch to switch to a different branch in the repository.
        case E_ARG_TYPE_SWITCH_BRANCH: {
            setup();
            return handle_switch_branch(args);
        };
        // -rB or --rebase-branch to rebase a branch onto anothers head.
        case E_ARG_TYPE_REBASE_BRANCH: {
            setup();
            return handle_rebase_branch(args);
        }
        // -cc or --clear-cache to clear any caches leftover from merges and or rebases.
        case E_ARG_TYPE_CLEAR_CACHE: {
            setup();
            return handle_clear_cache();
        }
        // -aT or --add-tag to add a tag to a commit
        case E_ARG_TYPE_ADD_TAG: {
            setup();
            return handle_add_tag(args);
        }
        // -dT or --delete-tag to delete a tag from the branches history.
        case E_ARG_TYPE_DELETE_TAG: {
            return handle_delete_tag(args);
        }

        // --h and --v have already been handled in <parse_args>.
        case E_ARG_TYPE_HELP:
        case E_ARG_TYPE_VERSION:
        default: {};
    }
    return 0;
};