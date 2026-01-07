/**
 * @author Sean Hobeck
 * @date 2026-01-06
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
#include "inw.h"

/*! @uses scan_object_cache */
#include "cache.h"

/*! @uses tag_t, create_tag, write_tag, ... */
#include "tag.h"

/*! @uses shelve_changes */
#include "shelve.h"

/*! @uses config_t, read_config */
#include "conf.h"

/*! @uses log, E_LOG_... */
#include "log.h"

/*! @uses a lot of things. */
#include "utl.h"

/* internal ptrs. */
internal repository_t* repository;
internal branch_t* active_branch;
internal config_t* config;

/* internal argument flags. */
internal bool all = false, no_recurse = false, hard = false, graph = false, \
    filter = false, max_count = false, verbose = false, quiet = false, from = false;

/* logging with the internal argument flags given. */
#define _llog(level, format, ...) if (!quiet) llog(level, format, ##__VA_ARGS__);

internal void
setup(dyna_t* array) {
    /* read our repository from disk. */
    repository = read_repository();
    assert(repository != 0x0);

    /* get the active branch. */
    active_branch = dyna_get(repository->branches, repository->idx);
    assert(active_branch != 0x0);

    /* read the config file. */
    config = read_config();

    /* parse for flag arguments. */
    _foreach(array, argument_t*, argument)
        if (argument->type == E_FLAG_TO_ARGUMENT) {
            /* we then set the internal flags AS specified. */
            switch (argument->details.flag) {
                case E_FLAG_ARG_TYPE_ALL: {
                    all = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_NO_RECURSE: {
                    no_recurse = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_HARD: {
                    hard = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_GRAPH: {
                    graph = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_FILTER: {
                    filter = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_MAX_COUNT: {
                    max_count = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_VERBOSE: {
                    verbose = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_QUIET: {
                    quiet = true;
                    break;
                }
                case E_FLAG_ARG_TYPE_FROM: {
                    from = true;
                    break;
                }
                default: {}
            }
        }
    _endforeach
}

internal int
handle_init() {
    /* create the repository, unless it has already been made. */
    if (create_repository() == 0) {
        llog(E_LOGGER_LEVEL_ERROR, "repository already exists.\n");
        return -1;
    }
    _llog(E_LOGGER_LEVEL_INFO, "repository initialized successfully.\n");
    return 0;
}

internal int
handle_log() {
    /* inode walk to collect all shelved files. */
    dyna_t* shelved_array = collect_shelved(active_branch->name);

    /* print out the active branch name and the number of diffs shelved. */
    printf("current branch: \'%s\', %lu change(s) shelved, with %lu commit(s), %s.\n", \
        active_branch->name, shelved_array->length, active_branch->commits->length,
        repository->readonly ? "in read-only " : "in read-write");

    /* iterate through each object. */
    _foreach(active_branch->commits, const commit_t*, commit)
        if (i == active_branch->head) printf("\t    ->  ");
        else printf("\t\t");

        /* print the information about the commits. */
        char* _hash = strsha1(commit->hash);
        printf("%s : %s @ %s\n", strtrm(_hash, 60), \
            strtrm(commit->message, 32), commit->timestamp);
        free(_hash);
    _endforeach;

    /* print out all the tags (on this branch). */
    dyna_t* tags = read_tags();
    tags = filter_tags(active_branch->hash, tags);
    if (tags->length > 0) {
        printf("\ntag(s):\n");
        _foreach(tags, tag_t*, tag)
            printf("\t\t%s -> %s\n", tag->name, strsha1(tag->commit_hash));
        _endforeach;
    }
    return 0;
}

internal int
handle_commit(dyna_t* argument_array) {
    /* if we are in read-only mode, we cannot commit or make changes */
    if (repository->readonly) {
        llog(E_LOGGER_LEVEL_ERROR, "cannot commit changes in read-only mode.\n");
        return -1;
    }

    /* inode walk and collect all shelved files. */
    dyna_t* shelved_array = collect_shelved(active_branch->name);

    /* create the commit with a message. */
    char* message = ".";
    _foreach_it(argument_array, const argument_t*, argument, i)
        /* if we encounter a --message, whatever follows must be a message in quotes. */
        if (argument->type == E_FLAG_TO_ARGUMENT) {
            if (argument->details.flag == E_FLAG_ARG_TYPE_MESSAGE) {
                const argument_t* next = _get(argument_array, argument_t*, i + 1);

                /* copy the message over. */
                message = strdup(next->value);
                break;
            }
        }
    _endforeach;
    commit_t* commit = create_commit(message, active_branch->name);

    /* if there are no changes|diffs made. */
    if (shelved_array->data == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR, "no diffs to commit; nothing stashed.\n");
        remove(commit->path);
        return -1;
    }

    /* iterate through each shelved item. */
    _foreach(shelved_array, const inode_t*, inode);
        /* read and add to the commit. */
        diff_t* read = read_diff(inode->path);
        dyna_push(commit->changes, read);
        remove(inode->path);
    _endforeach;
    dyna_free(shelved_array);

    /* add the commit to the active branch history. */
    dyna_push(active_branch->commits, commit);
    write_commit(commit);

    /* set the active|head commit index to the new commit and write. */
    active_branch->head = active_branch->commits->length - 1;
    write_branch(active_branch);

    /* remove the staging directory. */
    char path[256];
    snprintf(path, 256, ".lit/objects/shelved/%s", active_branch->name);
    remove(path);

    /* print out to the console. */
    _llog(E_LOGGER_LEVEL_INFO, "added commit '%s' to branch '%s' with %lu change(s).\n", \
        strtrm(commit->message, 32), active_branch->name, commit->changes->length);
    return 0;
}

internal int
handle_cr_move(dyna_t* argument_array) {
    /* read all the tags in the repository. */
    dyna_t* tags = read_tags();

    /* gather the hash or the tag. */
    sha1_t hash = {0};
    _foreach_it(argument_array, const argument_t*, argument, i)
        /* are we encountering a tag? */
        if (argument->type == E_FLAG_TO_ARGUMENT) {
            if (argument->details.flag == E_FLAG_ARG_TYPE_TAG) {
                const argument_t* next = _get(argument_array, const argument_t*, i + 1);

                /* check if there are any tags. */
                if (tags->length == 0) {
                    llog(E_LOGGER_LEVEL_ERROR, "no tags exist in the repository.\n");
                    return -1;
                }

                /* get the tag that matches the value of the parameter. */
                _foreach_it(tags, tag_t*, tag, j)
                    if (!strcmp(tag->name, next->value)) {
                        memcpy(hash, tag->commit_hash, 20);
                        break;
                    }
                _endforeach;
                dyna_free(tags);
                break;
            }
        }
        /* otherwise, are we encountering a hash. */
        else if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            memcpy(hash, strtoha(argument->value, 20), 20);
            break;
        }
    _endforeach;

    /* gather the specific commit ptr. */
    commit_t* target_commit = 0x0;
    size_t target_idx = 0;
    _foreach_it(active_branch->commits, commit_t*, commit, i)
        /* compare the hashes of the commit to see if they match. */
        if (!memcmp(commit->hash, hash, 20)) {
            target_commit = commit;
            target_idx = i;
            break;
        }
    _endforeach;

    /* if the commit was not found, report the error and return. */
    if (target_commit == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR, "commit '%s' not found.\n", strsha1(hash));
        return -1;
    }

    /* find the proper argument in the argument array, then perform the operation. */
    e_proper_arg_ty_t type = E_PROPER_ARG_TYPE_NONE;
    _foreach(argument_array, const argument_t*, argument)
        if (argument->type == E_PROPER_ARGUMENT) {
            type = argument->details.proper;
            break;
        }
    _endforeach;
    if (type == E_PROPER_ARG_TYPE_ROLLBACK) {
        /* if the commit is newer than the current commit, report the error and return. */
        if (target_idx >= active_branch->head) {
            llog(E_LOGGER_LEVEL_ERROR, "cannot rollback to a commit that is newer than the active commit.\n");
            return -1;
        }

        /* rollback to the commit by applying the diffs in reverse order. */
        rollback_op(active_branch, target_commit);
        if (!quiet) {
            llog(E_LOGGER_LEVEL_INFO, "rolled back to \'%s\' on branch \'%s\'\n", \
                strtrm(strsha1(target_commit->hash), 12), active_branch->name);
        }
    }
    else {
        /* if the commit is older than the current commit, report the error and return. */
        if (target_idx <= active_branch->head) {
            llog(E_LOGGER_LEVEL_ERROR, "cannot checkout to a commit that is older than the active commit.\n");
            return -1;
        }

        /* checkout to the commit by applying the diffs in order. */
        checkout_op(active_branch, target_commit);
        if (!quiet) {
            llog(E_LOGGER_LEVEL_INFO, "checked out \'%s\' on branch \'%s\'\n", \
                strtrm(strsha1(target_commit->hash), 12), active_branch->name);
        }
    }

    /* write and leave. */
    active_branch->head = target_idx;
    write_branch(active_branch);

    /* if we are not on the latest commit, set the repository to read-only. */
    repository->readonly = target_idx != active_branch->commits->length - 1;
    write_repository(repository);

    /* log a warning if verbose. */
    _llog(E_LOGGER_LEVEL_WARNING, "\e[0;33mwarning, treat rollbacks and checkouts as readonly.\n"
        "changing any files could damage your control tree.\n\e[0m");

    /* if the user specifies with --hard, delete all shelved items. */
    if (hard) {
        /* inode walk and collect all shelved files. */
        dyna_t* shelved_array = collect_shelved(active_branch->name);
        _foreach(shelved_array, inode_t*, inode)
            remove(inode->path);
        _endforeach;
        dyna_free(shelved_array);
    }
    return 0;
}

internal int
add_delete_inode(const char* filename, e_proper_arg_ty_t type) {
    /* if we are adding a file or folder, we need to create a diff for it. */
    diff_t* diff;
    if (filename[strlen(filename) - 1] == '/') {
        diff = create_folder_diff(filename,
            type == E_PROPER_ARG_TYPE_ADD_INODE ?
            E_DIFF_FOLDER_NEW : E_DIFF_FOLDER_DELETED);
    }
    else {
        diff = create_file_diff(filename,
            type == E_PROPER_ARG_TYPE_ADD_INODE ?
            E_DIFF_FILE_NEW : E_DIFF_FILE_DELETED);
    }

    /* if we are deleting a folder/file, we need to remove it. */
    if (type == E_PROPER_ARG_TYPE_DELETE_INODE) {
        remove(diff->stored_path);
    }

    /* error checking. */
    if (diff->new_path == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR, "failed to create diff for '%s'.\n", filename);
        return -1;
    }

    /* push to the shelf for the active branch. */
    write_to_shelved(active_branch->name, diff);
    return 0;
}

internal diff_t*
find_recent_commit(const char* filename) {
    /* iterate through all commits in this branch. */
    diff_t* recent_commit = 0x0;
    _inv_foreach_it(active_branch->commits, const commit_t*, commit, i)
        /* search for the file in the commit using its new_path first. */
        _foreach_it(commit->changes, diff_t*, change, j)
            /* if the new_path on a diff is the original file we are looking for, then we need to re-construct it. */
            if (change->new_path && !strcmp(change->new_path, filename)) {
                if (change->type == E_DIFF_FILE_MODIFIED || change->type == E_DIFF_FILE_NEW) {
                    recent_commit = change;
                    break;
                }
            }
        _endforeach;
    _endforeach;
    return recent_commit;
}

internal int
modified_inode(const char* old_filename, const char* new_filename) {
    /* what filename are we looking for? */
    if (old_filename[strlen(old_filename) - 1] == '/') {
        /*
         * if this is a folder that we are dealing with then we need
         *  to create a new diff for the folder, and then a delete diff
         *  for the folder.
         */
        if (new_filename != 0x0 && new_filename[strlen(new_filename) - 1] == '/') {
            /* create a diff to delete the old folder and then create a new one,
             * but first we have to check that the older one exists.
             */
            diff_t* new_folder = create_folder_diff(new_filename, E_DIFF_FOLDER_NEW);
            diff_t* old_folder = create_folder_diff(old_filename, E_DIFF_FOLDER_DELETED);

            /* check if the folders exist. */
            struct stat osb;
            if (stat(old_folder->stored_path, &osb) != 0) {
                llog(E_LOGGER_LEVEL_ERROR, "stat failed; old folder not found.\n");
                return -1;
            }

            /* write the changes to be shelved. */
            write_to_shelved(active_branch->name, new_folder);
            write_to_shelved(active_branch->name, old_folder);

            /* log and rename the folder. */
            if (!quiet) {
                llog(E_LOGGER_LEVEL_INFO, "added changes for '%s' -> '%s' to stashed\n",
                       old_filename, new_filename);
            }
            rename(old_filename, new_filename);
            return 0;
        }

        /* print an error. */
        llog(E_LOGGER_LEVEL_ERROR, "cannot modify a folder to be a file, or write a .diff for a folder that hasn't been renamed.\n");
        return -1;
    }

    /* iterate to find the most recent commit on the active branch. */
    diff_t* most_recent_change = find_recent_commit(old_filename);

    /* did we find the most recent change that contains the file as new_path or stored_path? */
    if (!most_recent_change) {
        llog(E_LOGGER_LEVEL_ERROR, "file not found in previous commits on this branch.\n");
        return -1;
    }

    /* create a temporary file with the original content. */
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), ".lit/%ld.tmp", time(0x0));

    /* reconstruct the original file from the diff */
    size_t line_count = 0;
    if (most_recent_change->lines->length != 0 && most_recent_change->lines != 0x0) {
        /* clean the lines in the recent diff for reading. */
        char** original_lines = fcleanls((char**) most_recent_change->lines->data, \
            most_recent_change->lines->length, &line_count);
        if (!original_lines) {
            llog(E_LOGGER_LEVEL_ERROR, "failed to reconstruct original file content.\n");
            return -1;
        }

        /* write original content to .tmp file. */
        fwritels(temp_path, original_lines, line_count);
    }
    else {
        /* write original content to .tmp file. */
        FILE* f = fopen(temp_path, "w");
        if (!f) {
            llog(E_LOGGER_LEVEL_ERROR, "fopen failed; could not open temp file for writing.\n");
            return -1;
        }
        fclose(f);
    }

    /* create the modified diff between original and active. */
    diff_t* diff = create_file_modified_diff(temp_path, new_filename);
    diff->stored_path = strdup(old_filename); /* set the stored path to the original file (for rollback purposes) */

    /* clean up .tmp file and allocated lines. */
    remove(temp_path);
    write_to_shelved(active_branch->name, diff);
    return 0;
}

internal int
handle_add(dyna_t* argument_array) {
    /* if we are in read-only mode, we cannot commit or make changes. */
    if (repository->readonly) {
        llog(E_LOGGER_LEVEL_ERROR, "cannot commit changes in read-only mode.\n");
        return -1;
    }

    /* is there a recursive folder for us to look through? */
    if (all || no_recurse) {
        _foreach_it(argument_array, const argument_t*, argument, i)
            if (argument->type == E_FLAG_TO_ARGUMENT) {
                /* get the following parameter argument. */
                const argument_t* next = _get(argument_array, const argument_t*, i + 1);

                /* we then perform an inode walk on the folder provided. */
                dyna_t* inodes = inw_walk(next->value, all ? E_INW_TYPE_RECURSE : E_INW_TYPE_NO_RECURSE);

                /* iterate through each inode and add it. */
                _foreach_it(inodes, inode_t*, inode, j)
                    /* we then need to check if there are any commits before that contain this inode at all. */
                    bool is_new_file = find_recent_commit(inode->name) != 0x0;
                    if (is_new_file) {
                        if (modified_inode(inode->path, inode->name) == -1)
                            return -1;
                    }
                    else if (add_delete_inode(inode->path, E_PROPER_ARG_TYPE_ADD_INODE) == -1)
                        return -1;
                _endforeach;
                dyna_free(inodes);
                break;
            }
        _endforeach;
    }
    else {
        /* otherwise we are adding a single file or folder. */
        char* filename = 0x0;
        _foreach(argument_array, const argument_t*, argument)
            if (argument->type == E_PARAMETER_TO_ARGUMENT)
                filename = strdup(argument->value);
        _endforeach;

        /* we add the single folder and we are done. */
        if (filename == 0x0)
            return -1;
        diff_t* diff = find_recent_commit(filename);
        bool is_new_file = diff != 0x0;
        if (is_new_file) {
            if (modified_inode(diff->stored_path, filename) == -1)
                return -1;
        }
        else if (add_delete_inode(filename, E_PROPER_ARG_TYPE_ADD_INODE) == -1)
            return -1;
    }
    return 0;
}

internal int
handle_delete(dyna_t* argument_array) {
    /* if we are in read-only mode, we cannot commit or make changes. */
    if (repository->readonly) {
        llog(E_LOGGER_LEVEL_ERROR, "cannot commit changes in read-only mode.\n");
        return -1;
    }

    /* is there a recursive folder for us to look through? */
    if (all || no_recurse) {
        _foreach_it(argument_array, const argument_t*, argument, i)
            if (argument->type == E_FLAG_TO_ARGUMENT) {
                /* get the following parameter argument. */
                const argument_t* next = _get(argument_array, argument_t*, i + 1);

                /* we then perform an inode walk on the folder provided. */
                dyna_t* inodes = inw_walk(next->value, all ? E_INW_TYPE_RECURSE : E_INW_TYPE_NO_RECURSE);

                /* iterate through each inode and add it. */
                _foreach_it(inodes, inode_t*, inode, j)
                    /* we then add the deleted item. */
                    add_delete_inode(inode->path, E_PROPER_ARG_TYPE_DELETE_INODE);
                _endforeach;
                dyna_free(inodes);
                break;
            }
        _endforeach;
    }
    else {
        /* otherwise we are removing a single file or folder. */
        char* filename = 0x0;
        _foreach(argument_array, const argument_t*, argument)
            if (argument->type == E_PARAMETER_TO_ARGUMENT)
                filename = strdup(argument->value);
        _endforeach;

        /* we add the deleted item and we are done. */
        if (filename == 0x0)
            return -1;
        add_delete_inode(filename, E_PROPER_ARG_TYPE_DELETE_INODE);
    }
    return 0;
}

internal int
handle_create_branch(dyna_t* argument_array) {
    /* find the name and the possible --from branch name. */
    char* branch_name = 0x0, *from_branch_name = 0x0;
    _foreach_it(argument_array, const argument_t*, argument, i)
        if (argument->type == E_FLAG_TO_ARGUMENT) {
            if (argument->details.flag == E_FLAG_ARG_TYPE_FROM) {
                const argument_t* next = _get(argument_array, argument_t*, i + 1);
                from_branch_name = strdup(next->value);
            }
        }
        else if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            branch_name = strdup(argument->value);
        }
    _endforeach;

    /* run the operation. */
    if (from_branch_name == 0x0)
        from_branch_name = strdup(active_branch->name);
    create_branch_repository(repository, branch_name, from_branch_name);

    /* print out to the console. */
    _llog(E_LOGGER_LEVEL_INFO, "created branch '%s' from '%s'.\n", branch_name, from_branch_name);
    return 0;
}

internal int
handle_delete_branch(dyna_t* argument_array) {
    /* find the name for the branch. */
    char* branch_name = 0x0;
    _foreach(argument_array, argument_t*, argument)
        if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            branch_name = strdup(argument->value);
            break;
        }
    _endforeach;

    /* run the operation. */
    delete_branch_repository(repository, branch_name);

    /* switching back to the origin branch. */
    _llog(E_LOGGER_LEVEL_INFO, "deleted branch '%s'.\n", branch_name);
    if (!strcmp(active_branch->name, branch_name)) {
        _llog(E_LOGGER_LEVEL_INFO, "switching back to origin branch.\n");
        switch_branch_repository(repository, "origin");
    }

    /* write the repository out to the file. */
    write_repository(repository);
    return 0;
}

internal int
handle_switch_branch(dyna_t* argument_array) {
    /* find the name for the branch. */
    char* branch_name = 0x0;
    _foreach(argument_array, argument_t*, argument)
        if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            branch_name = strdup(argument->value);
            break;
        }
    _endforeach;

    /* run the operation. */
    switch_branch_repository(repository, branch_name);
    _llog(E_LOGGER_LEVEL_INFO, "switched to branch '%s'.\n", branch_name);
    return 0;
}

internal int
handle_rebase_branch(dyna_t* argument_array) {
    /* find the branch that we are going to rebase onto. */
    char* source_branch_name = 0x0, *destination_branch_name = 0x0;
    _foreach(argument_array, argument_t*, argument)
        if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            if (source_branch_name == 0x0) {
                source_branch_name = strdup(argument->value);
            }
            else {
                destination_branch_name = strdup(argument->value);
                break;
            }
        }
    _endforeach;
    if (destination_branch_name == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR, "destination branch not specified.\n");
        exit(-1);
    }

    /* rebase onto the branch provided. */
    return branch_rebase(repository, destination_branch_name, \
        source_branch_name) == E_REBASE_RESULT_SUCCESS ? 0 : 1;
}

internal int
handle_clear_cache() {
    /* scan the object cache and clear it. */
    return scan_object_cache(repository) == E_CACHE_RESULT_SUCCESS ? 0 : 1;
}

internal int
handle_add_tag(dyna_t* argument_array) {
    /* gather the user hash. */
    unsigned char* hash = 0x0;
    char* tag_name = 0x0;
    _foreach_it(argument_array, const argument_t*, argument, i)
        if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            /* we expect the first parameter to be the hash, the second to be the tag name. */
            const argument_t* next = _get(argument_array, const argument_t*, i + 1);
            hash = strtoha(argument->value, 20);
            tag_name = strdup(next->value);
            break;
        }
    _endforeach;

    /* iterate over the changes. */
    commit_t* commit = 0x0;
    _foreach(active_branch->commits, commit_t*, _commit)
        /* memcmp the sha1 hashes. */
        if (memcmp(hash, _commit->hash, 20) == 0) {
            commit = _commit;
            break;
        }
    _endforeach;

    /* report the fatal error if a commit was not found. */
    if (!commit) {
        llog(E_LOGGER_LEVEL_ERROR, "did not find commit hash \'%s\' in active "
                        "branches history.\n", tag_name);
        return -1;
    }

    /* create the tag given the hash and the name. */
    tag_t* tag = create_tag(active_branch, commit, tag_name);
    write_tag(tag);
    _llog(E_LOGGER_LEVEL_INFO, "added tag \'%s\' to the repository.\n", tag_name);
    return 0;
}

internal int
handle_delete_tag(dyna_t* argument_array) {
    /* read all the tags. */
    dyna_t* tag_array = read_tags();

    /* find the tag name */
    char* tag_name = 0x0;
    _foreach(argument_array, const argument_t*, argument)
        if (argument->type == E_PARAMETER_TO_ARGUMENT) {
            tag_name = strdup(argument->value);
        }
    _endforeach;

    /* if we did not find the tag name. */
    if (tag_name == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR, "tag name not found.\n");
        return -1;
    }

    /* check if the tag exists. */
    bool found = false;
    _foreach(tag_array, const tag_t*, tag)
        if (!strcmp(tag->name, tag_name))
            found = true;
    _endforeach;

    /* if we did not find the tag. */
    if (!found) {
        llog(E_LOGGER_LEVEL_ERROR, "tag \'%s\' not found.\n", tag_name);
        return -1;
    }

    /* remove the path. */
    char path[256];
    snprintf(path, 256, ".lit/refs/tags/%s", tag_name);
    remove(path);

    /* log and return. */
    _llog(E_LOGGER_LEVEL_INFO, "deleted tag \'%s\' from the repository.\n", tag_name);
    return 0;
}

/**
 * @brief handle the arguments passed by the user as a cli (command-line interface) tool.
 *
 * @param argument_array the dynamic array of arguments passed by the user.
 * @return the return code if needed (0 if not required).
 */
int
cli_handle(dyna_t* argument_array) {
    /* iterate through each argument and handle it. */
    e_proper_arg_ty_t proper_type = E_PROPER_ARG_TYPE_NONE;
    _foreach(argument_array, argument_t*, argument)
        /* if we encounter the proper argument, then we set the type. */
        if (argument->type == E_PROPER_ARGUMENT)
            proper_type = argument->details.proper;
    _endforeach;

    /* handle each proper type variant. */
    switch (proper_type) {
        /* -i | init to initialize a new repository. */
        case E_PROPER_ARG_TYPE_INIT: {
            return handle_init();
        }
        /* -l | log to show the status of the repository. */
        case E_PROPER_ARG_TYPE_LOG: {
            setup(argument_array);
            return handle_log();
        }
        /* -c | commit to add changes to the repository. */
        case E_PROPER_ARG_TYPE_COMMIT: {
            setup(argument_array);
            return handle_commit(argument_array);
        }
        /* -C | checkout to go forward to a newer commit. */
        case E_PROPER_ARG_TYPE_CHECKOUT:
        /* -r | --rollback to go back to a previous commit. */
        case E_PROPER_ARG_TYPE_ROLLBACK: {
            setup(argument_array);
            return handle_cr_move(argument_array);
        }
        /* -a | add to add a file or folder to the repository. */
        case E_PROPER_ARG_TYPE_ADD_INODE: {
            setup(argument_array);
            return handle_add(argument_array);
        }
        /* -d | delete to delete a file or folder from the repository. */
        case E_PROPER_ARG_TYPE_DELETE_INODE: {
            setup(argument_array);
            return handle_delete(argument_array);
        }
        /* -aB | add-branch to create a new branch off of the origin branches head. */
        case E_PROPER_ARG_TYPE_CREATE_BRANCH: {
            setup(argument_array);
            return handle_create_branch(argument_array);
        }
        /* -dB | delete-branch to delete a branch from the repository. */
        case E_PROPER_ARG_TYPE_DELETE_BRANCH: {
            setup(argument_array);
            return handle_delete_branch(argument_array);
        }
        /* -sB | switch-branch to switch to a different branch in the repository. */
        case E_PROPER_ARG_TYPE_SWITCH_BRANCH: {
            setup(argument_array);
            return handle_switch_branch(argument_array);
        }
        /* -rB | rebase-branch to rebase a branch onto another branch (not a squash). */
        case E_PROPER_ARG_TYPE_REBASE_BRANCH: {
            setup(argument_array);
            return handle_rebase_branch(argument_array);
        }
        /* -cc | clear-cache to clear any caches leftover. */
        case E_PROPER_ARG_TYPE_CLEAR_CACHE: {
            setup(argument_array);
            return handle_clear_cache();
        }
        /* -aT | add-tag to add a tag to the repository for a specified commit. */
        case E_PROPER_ARG_TYPE_ADD_TAG: {
            setup(argument_array);
            return handle_add_tag(argument_array);
        }
        /* -dT | delete-tag to delete a tag from the repository. */
        case E_PROPER_ARG_TYPE_DELETE_TAG: {
            return handle_delete_tag(argument_array);
        }
        default: {}
    }
    return 0;
}