/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#include "repo.h"

/*! @uses assert. */
#include <assert.h>

/*! @uses mkdir, getcwd. */
#include <sys/stat.h>

/*! @uses printf, fprintf, perror, fopen, fclose, fscanf. */
#include <stdio.h>

/*! @uses strcpy, strncpy. */
#include <string.h>

/*! @uses malloc, free. */
#include <stdlib.h>

/*! @uses getcwd, chdir */
#include <unistd.h>

/*! @uses apply_inverse_commit, apply_forward_commit */
#include "ops.h"

/*! @uses MKDIR_MOWNER. */
#include "utl.h"

/*! @uses llog, E_LOGGER_LEVEL_ERROR. */
#include "log.h"

/**
 * @brief find a common commit ancestor using hashes and timestamps by going backwards.
 *
 * @param branch1 the first branch.
 * @param branch2 the second branch.
 * @return pointer to the common ancestor commit, or 0x0 if none found.
 */
commit_t*
find_common_ancestor(branch_t* branch1, branch_t* branch2) {
    /* is there any issues with the commits provided? */
    assert(branch1 != 0x0);
    assert(branch2 != 0x0);
    if (branch1->commits->length == 0 || branch2->commits->length == 0)
        return 0x0;

    /* start from the most recent comits and work backwards */
    long i = (long) branch1->commits->length - 1;
    long j = (long) branch2->commits->length - 1;
    commit_t* ancestor = 0x0;

    /* work backwards through both branches simultaneously */
    while (i >= 0 && j >= 0) {
        commit_t* c1 = dyna_get(branch1->commits, i), *c2 = dyna_get(branch2->commits, j);

        /* if hashes match, this a common commit. */
        if (memcmp(c1->hash, c2->hash, 32) == 0) {
            ancestor = c1;
            break;
        }
        /* move the pointer with the more recent timestamp backwards. */
        if (c1->timestamp > c2->timestamp) i--;
        else j--;
    }
    return ancestor;
}

/**
 * @brief find the index of a commit in a branch.
 *
 * @param branch the branch to be searched.
 * @param commit the commit to find.
 * @return the index of the commit or -1 if not found.
 */
long
find_index_commit(branch_t* branch, commit_t* commit) {
    /* assert the branch and the commit. */
    assert(branch);
    assert(commit);
    _foreach_it(branch->commits, const commit_t*, _commit, i)
        if (memcmp(_commit->hash, commit->hash, 32) == 0) {
            return (long) i;
        }
    _endforeach;
    return -1;
};

/**
 * @brief create a new repository with the given main branch name.
 *
 * @return a 0 if successful in creating the directory, or -1 if .lit directory already exists.
 */
repository_t*
create_repository() {
    /* get the cwd. */
    char cwd[256];
    if (getcwd(cwd, sizeof cwd) == 0x0) {
        llog(E_LOGGER_LEVEL_ERROR,"getcwd failed; could not get current working directory.\n");
        exit(EXIT_FAILURE);
    }

    /* set the cwd. */
    if (chdir(cwd) != 0) {
        llog(E_LOGGER_LEVEL_ERROR,"chdir failed; could not change to current working directory.\n");
        exit(EXIT_FAILURE);
    }

    /* create the '.lit' directory in the current working directory. */
    if (mkdir(".lit", MKDIR_MOWNER) == -1) {
        llog(E_LOGGER_LEVEL_ERROR, "mkdir failed; '.lit' directory already exists.\n");
        exit(EXIT_FAILURE);
    }
    /* content-addressable storage folders. */
    mkdir(".lit/objects", MKDIR_MOWNER);
    mkdir(".lit/objects/commits", MKDIR_MOWNER);
    mkdir(".lit/objects/diffs", MKDIR_MOWNER);

    /* references (branches, tags). */
    mkdir(".lit/refs", MKDIR_MOWNER);
    mkdir(".lit/refs/heads/", MKDIR_MOWNER);
    mkdir(".lit/refs/tags/", MKDIR_MOWNER);

    /* create a new repository structure. */
    repository_t* repo = calloc(1, sizeof(*repo));
    repo->branches = dyna_create(sizeof(branch_t*));
    dyna_push(repo->branches, create_branch("origin"));
    repo->readonly = false;

    /* write the repository and branch to disk. */
    write_branch(dyna_get(repo->branches, 0));
    write_repository(repo);
    return repo;
};

/**
 * @brief write the repository to disk in our '.lit' directory.
 *
 * @param repo the repository_t structure to be written to a file.
 */
void
write_repository(const repository_t* repo) {
    /* assert the repository. */
    assert(repo != 0x0);

    /* open the file '.lit/repository' for writing. */
    FILE* f = fopen(".lit/index", "w");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open index file for writing.\n");
        exit(EXIT_FAILURE);
    }

    /* write the main branch information. */
    fprintf(f, "active:%lu\n", repo->idx);
    fprintf(f, "count:%lu\n", repo->branches->length);
    fprintf(f, "readonly:%d\n", repo->readonly ? 1 : 0);
    _foreach_it(repo->branches, const branch_t*, branch, i)
        fprintf(f, "%lu:%s\n", i, branch->name);
    _endforeach;
    fclose(f);
};

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t*
read_repository() {
    /* create a temporary repository structure. */
    repository_t* repo = calloc(1, sizeof *repo);

    /* open the file '.lit/index' for reading. */
    FILE* f = fopen(".lit/index", "r");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open repository file for reading.\n");
        exit(EXIT_FAILURE);
    }

    /* read the current branch index. */
    size_t length = 0;
    int scanned = fscanf(f, "active:%lu\ncount:%lu\nreadonly:%d", &repo->idx, \
        &length, &repo->readonly);
    if (scanned != 3) {
        llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read current branch header.\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    /* if there are no branches, return the repository. */
    if (length == 0u) {
        repo->branches = 0x0;
        fclose(f);
        return repo;
    }

    /* create the dynamic array. */
    repo->branches = dyna_create(sizeof(branch_t*));

    /* iterate through the count. */
    for (size_t i = 0; i < length; i++) {
        /* allocate and read. */
        char* branch_name = calloc(1, 129);
        scanned = fscanf(f, "%lu:%128[^\n]\n", &i, branch_name);
        if (scanned != 2) {
            llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read branch name.\n");
            fclose(f);
            free(repo->branches);
            return repo;
        }

        /* read the branch from refs/heads/ */
        char *path = calloc(1, 257);
        snprintf(path, 256, ".lit/refs/heads/%s", branch_name);
        dyna_push(repo->branches, read_branch(branch_name));
        free(path);
        free(branch_name);
    };
    return repo;
};

/**
 * @brief create a new branch from the current branches HEAD commit.
 *
 * @param repository the repository provided.
 * @param name the name of the new branch.
 * @param from_name the name of the branch to copy the head from.
 */
void
create_branch_repository(repository_t* repository, const char* name, const char* from_name) {
    /* assert the repository and the name. */
    assert(repository != 0x0);
    assert(name != 0x0);

    /* check if the branch exists. */
    _foreach(repository->branches, const branch_t*, branch)
        if (!strcmp(branch->name, name)) {
            llog(E_LOGGER_LEVEL_ERROR,"strcmp; branch \'%s\' already exists.\n", name);
            exit(EXIT_FAILURE);
        }
    _endforeach;

    /* create the branch. */
    branch_t* branch = create_branch(name);
    if (!branch) {
        llog(E_LOGGER_LEVEL_ERROR,"branch_create failed; could not create branch of name \'%s\'\n", name);
        exit(EXIT_FAILURE);
    }

    /* add the branch to this repository. */
    dyna_push(repository->branches, branch);

    /* get the from the branch specified. */
    branch_t* from_branch = 0x0;
    _foreach(repository->branches, branch_t*, _branch)
        /* search by name. */
        if (!strcmp(_branch->name, from_name)) {
            from_branch = _branch;
            break;
        }
    _endforeach;

    /* check the from branch exists. */
    if (!from_branch) {
        llog(E_LOGGER_LEVEL_ERROR,"strcmp; branch \'%s\' does not exist.\n", from_name);
        exit(EXIT_FAILURE);
    }

    /* copy all the commits over to the new branch. */
    _foreach(from_branch->commits, commit_t*, commit)
        commit_t* _commit = calloc(1, sizeof *_commit);
        memcpy(_commit, commit, sizeof *_commit);

        /* keep the sha1 hash on the commit, if we delete a branch
         *  and the changes aren't kept then we simply just removed
         *  them as they are just 'cache'. */
        dyna_push(branch->commits, commit);
    _endforeach;
    branch->head = from_branch->head;

    /* write out the branch and repository. */
    write_repository(repository);
    write_branch(branch);
};

/**
 * @brief delete a branch from the repository.
 *
 * @param repository the repository provided.
 * @param name the name of the branch.
 */
void
delete_branch_repository(repository_t* repository, const char* name) {
    /* assert the repository and the name. */
    assert(repository != 0x0);
    assert(name != 0x0);

    /* we cannot delete the original/ origin branch. */
    if (!strcmp(name, "origin")) {
        llog(E_LOGGER_LEVEL_ERROR,"strcmp; branch name cannot be 'origin'.\n");
        exit(EXIT_FAILURE);
    }

    /* check if the branch exists. */
    size_t i = (size_t) -1;
    _foreach_it(repository->branches, branch_t*, branch, j)
        if (!strcmp(branch->name, name)) {
            i = j;
            break;
        }
    _endforeach;
    if (i == (size_t) -1) {
        llog(E_LOGGER_LEVEL_ERROR,"strcmp; branch does not exist.\n");
        exit(EXIT_FAILURE);
    }

    /* remove the branch directory. */
    char path[256];
    snprintf(path, 256, ".lit/refs/heads/%s", name);
    remove(path);

    /* pop and move the branches down (if there are any). */
    dyna_pop(repository->branches, i);
};

/**
 * @brief get the branch from the repository with error checking.
 *
 * @param repository the repository read from the cwd.
 * @param branch_name the name of the branch to get.
 * @return a branch_t* if the branch was found, and EXIT_FAILURE if it wasn't.
 */
branch_t*
get_branch_repository(const repository_t* repository, const char* branch_name) {
    /* assert the repository and the branch name. */
    assert(repository != 0x0);
    assert(branch_name != 0x0);

    /* iterate through each branch in the repository. */
    _foreach_it(repository->branches, branch_t*, branch, i)
        if (!strcmp(branch->name, branch_name)) {
            return dyna_get(repository->branches, i);
        }
    _endforeach;

    /* if the target branch was not found. */
    llog(E_LOGGER_LEVEL_ERROR, "target branch \'%s\' not found.\n", branch_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief switch to the branch provided.
 *
 * @param repository the repository provided.
 * @param name the name of the new branch.
 */
void
switch_branch_repository(repository_t* repository, const char* name) {
    /* assert the repository and the new branch name. */
    assert(repository != 0x0);
    assert(name != 0x0);

    /* find which branch we are going to switch to. */
    branch_t* target = 0x0;
    size_t target_idx = 0;

    /* find the branch in the repositories branches. */
    _foreach_it(repository->branches, branch_t*, branch, i)
        if (!strcmp(branch->name, name)) {
            target = branch;
            target_idx = i;
            break;
        }
    _endforeach;

    /* if we did not find the target branch, exit(-1). */
    if (!target) {
        llog(E_LOGGER_LEVEL_ERROR,"strcmp; branch does not exist.\n");
        exit(EXIT_FAILURE);
    }

    /* if we are already on the branch, do nothing. */
    if (repository->idx == target_idx) {
        printf("already on branch \'%s\'.\n", name);
        exit(0);
    }

    /* find the common ancestor of the branch via timestamp, then
     *  do a checkout or rollback based on the direction that we are
     *  heading with this commit. */
    branch_t* current = dyna_get(repository->branches, repository->idx);
    commit_t* ancestor = find_common_ancestor(current , target);
    if (!ancestor) {
        /* this should NOT happen, warn the user. */
        printf("warning; no ancestor commit was found (branch is unrelated).\n");
        if (current->commits->length > 0) {
            /* rollback to the first commit. */
            rollback_op(current, dyna_get(current->commits, 0));
            if (current->commits->length > 0)
                reverse_commit_op(dyna_get(current->commits, 0));
        }

        /* checkout target branch to its head. */
        if (target->commits->length > 0) {
            /* start from clean slate and apply all commits. */
            for (size_t i = 0; i <= target->head && i < target->commits->length; i++)
                forward_commit_op(dyna_get(target->commits, i));
        }
    }
    else {
        /* do a switch, rollback to the ancestor, and then checkout the target commit. */
        printf("ancestor commit found; %s\n", strtrm(strsha1(ancestor->hash), 15));

        /* rollback to the ancestor commit... */
        long ancestor_idx = find_index_commit(current, ancestor);
        if (ancestor_idx >= 0 && ancestor_idx < current->head)
            for (size_t i = current->head; i > ancestor_idx; i--)
                reverse_commit_op(dyna_get(current->commits, i));

        /* checkout to the ancestor commit, then to the target head. */
        long head_idx = find_index_commit(target, ancestor);
        if (head_idx >= 0) {
            /* apply forward commits from the ancestor to the head. */
            for (long i = head_idx + 1; i <= (long) target->head && i < (long)
                target->commits->length; i++)
                forward_commit_op(dyna_get(target->commits, i));
        }
    }

    /* update the repository. */
    repository->idx = target_idx;
    write_repository(repository);
};