/**
 * @author Sean Hobeck
 * @date 2025-08-13
 *
 * @file repo.c
 *    the repository module in the version control system, it is responsible for handling branches,
 *    and their locations, as well as the overall state of the repository.
 */
#include "repo.h"

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

/**
 * @brief find a common commit ancestor using hashes and timestamps by going backwards.
 *
 * @param branch1 the first branch.
 * @param branch2 the second branch.
 * @return pointer to the common ancestor commit, or 0x0 if none found.
 */
commit_t*
find_common_ancestor(branch_t* branch1, branch_t* branch2) {
    // is there any issues with the commits provided?
    if (!branch1 || !branch2 || branch1->count == 0 || branch2->count == 0)
        return 0x0;

    // start from the most recent comits and work backwards
    long i = (long) branch1->count - 1;
    long j = (long) branch2->count - 1;
    commit_t* ancestor = 0x0;

    // work backwards through both branches simultaneously
    while (i >= 0 && j >= 0) {
        commit_t* c1 = branch1->commits[i], *c2 = branch2->commits[j];

        // if hashes match, this a common commit.
        if (memcmp(c1->hash, c2->hash, 32) == 0) {
            ancestor = c1;
            break;
        }
        // move the pointer with the more recent timestamp backwards.
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
    for (size_t i = 0; i < branch->count; i++) {
        if (memcmp(branch->commits[i]->hash, commit->hash, 32) == 0) {
            return (long) i;
        }
    }
    return -1;
};

/**
 * @brief dynamic list append the branch to the repository.
 *
 * @param branch the branch to be added to <repository>.
 * @param repository the repository where <branch> should be added.
 */
void
add_branch_to_repository(branch_t* branch, repository_t* repository) {
    if (repository->count >= repository->capacity) {
        repository->capacity = repository->capacity ? repository->capacity * 2 : 1; // increment the branch count.
        repository->branches = realloc(repository->branches, sizeof(branch_t*) * repository->capacity);
        if (!repository->branches) {
            fprintf(stderr,"realloc failed; could not allocate memory for branches.\n");
            exit(EXIT_FAILURE);
        }
    }
    repository->branches[repository->count++] = branch;
}

/**
 * @brief create a new repository with the given main branch name.
 *
 * @return a 0 if successful in creating the directory, or -1 if .lit directory already exists.
 */
repository_t*
create_repository() {
    // get the cwd.
    char cwd[256];
    if (getcwd(cwd, sizeof cwd) == 0x0) {
        fprintf(stderr,"getcwd failed; could not get current working directory.\n");
        exit(EXIT_FAILURE);
    }

    // set the cwd.
    if (chdir(cwd) != 0) {
        fprintf(stderr,"chdir failed; could not change to current working directory.\n");
        exit(EXIT_FAILURE);
    }

    // create the '.lit' directory in the current working directory.
    if (mkdir(".lit", 0755) == -1) {
        fprintf(stderr, "mkdir failed; '.lit' directory already exists.\n");
        exit(EXIT_FAILURE);
    }
    // content-addressable storage folders.
    mkdir(".lit/objects", 0755);
    mkdir(".lit/objects/commits", 0755);
    mkdir(".lit/objects/diffs", 0755);

    // references (branches, tags).
    mkdir(".lit/refs", 0755);
    mkdir(".lit/refs/heads/", 0755);
    mkdir(".lit/refs/tags/", 0755);

    // create a new repository structure.
    repository_t* repo = calloc(1, sizeof(*repo));
    add_branch_to_repository(create_branch("origin"), repo);

    // write the repository and branch to disk.
    write_branch(repo->branches[0]);
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
    // open the file '.lit/repository' for writing.
    FILE* f = fopen(".lit/index", "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open index file for writing.\n");
        exit(EXIT_FAILURE);
    }

    // write the main branch information.
    fprintf(f, "active:%lu\n", repo->idx);
    fprintf(f, "count:%lu\n", repo->count);
    for (size_t i = 0u; i < repo->count; i++)
        fprintf(f, "%lu:%s\n", i, repo->branches[i]->name);
    fclose(f);
};

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t*
read_repository() {
    // create a temporary repository structure.
    repository_t* repo = calloc(1, sizeof *repo);

    // open the file '.lit/repository' for reading.
    FILE* f = fopen(".lit/index", "r");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open repository file for reading.\n");
        exit(EXIT_FAILURE);
    }

    // read the current branch index.
    int scanned = fscanf(f, "active:%lu\ncount:%lu\n", &repo->idx, &repo->count);
    if (scanned != 2) {
        fprintf(stderr,"fscanf failed; could not read current branch header.\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    // if there are no branches, return the repository.
    if (repo->count == 0u) {
        repo->branches = 0x0;
        fclose(f);
        return repo;
    }

    // write the branches to the repository structure.
    repo->branches = calloc(1, sizeof(branch_t*) * repo->count);
    if (!repo->branches) {
        fprintf(stderr,"malloc failed; could not allocate memory for branches.\n");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    // iterate through the count.
    for (size_t i = 0u; i < repo->count; i++) {
        // allocate and read.
        char* branch_name = calloc(1, 128);
        scanned = fscanf(f, "%lu:%127[^\n]\n", &i, branch_name);
        if (scanned != 2) {
            fprintf(stderr,"fscanf failed; could not read branch name.\n");
            fclose(f);
            free(repo->branches);
            return repo;
        }

        /// read the branch from refs/heads/
        char *path = calloc(1, 256);
        snprintf(path, 256, ".lit/refs/heads/%s", branch_name);
        repo->branches[i] = read_branch(branch_name);
        free(branch_name);
    };
    return repo;
};

/**
 * @brief create a new branch from the current branches HEAD commit.
 *
 * @param repository the repository provided.
 * @param name the name of the new branch.
 */
void
create_branch_repository(repository_t* repository, const char* name) {
    // check if the branch exists.
    for (size_t i = 0; i < repository->count; i++) {
        if (!strcmp(repository->branches[i]->name, name)) {
            fprintf(stderr,"strcmp; branch \'%s\' already exists.\n", name);
            exit(EXIT_FAILURE);
        }
    }

    // create the branch,
    branch_t* branch = create_branch(name);
    if (!branch) {
        fprintf(stderr,"branch_create failed; could not create branch of name \'%s\'\n", name);
        exit(EXIT_FAILURE);
    }

    // add the branch to this repository.
    if (repository->count >= repository->capacity) {
        repository->capacity = repository->capacity ? repository->capacity * 2 : 1; // increment the branch count.
        repository->branches = realloc(repository->branches, sizeof(branch_t*) * repository->capacity);
        if (!repository->branches) {
            fprintf(stderr,"realloc failed; could not allocate memory for branches.\n");
            exit(EXIT_FAILURE);
        }
    }
    repository->branches[repository->count++] = branch;

    // get the current branch, and copy all of the commits from the top to the branch.
    branch_t* current = get_active_branch_repository(repository);
    for (size_t i = 0; i < current->count; i++) {
        commit_t* commit = calloc(1, sizeof *commit);
        memcpy(commit, current->commits[i], sizeof *commit);

        // keep the sha1 hash on the commit, if we delete a branch
        // and the changes aren't kept then we simply just removed
        // them as they are just 'cache'.
        add_commit_branch(commit, branch);
    }
    branch->idx = current->idx;

    // write out the branch and repository.
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
    // check if the branch exists.
    size_t i = 0;
    for (; i < repository->count; i++) {
        if (!strcmp(repository->branches[i]->name, name))
            break;
    }
    if (!strcmp(name, "origin")) {
        fprintf(stderr,"strcmp; branch name cannot be 'origin'.\n");
        exit(EXIT_FAILURE);
    }
    if (i == repository->count) {
        fprintf(stderr,"strcmp; branch does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // remove the branch directory.
    char path[256];
    snprintf(path, 256, ".lit/refs/heads/%s", name);
    remove(path);

    // move the branches down (if there are any)
    for (size_t j = i; j < repository->count - 1; j++)
        repository->branches[j] = repository->branches[j + 1];
    repository->count--;
};

/**
 * @brief get the current branch checked out for this repository.
 *
 * @param repository the repository provided.
 * @return the current branch checked out for this repository.
 */
branch_t*
get_active_branch_repository(repository_t* repository) {
    // if the index is -1, return main, else return the branch at the current index.
    return repository->branches[repository->idx];
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
    // iterate through each branch in the repository.
    for (size_t i = 0; i < repository->count; i++) {
        if (!strcmp(repository->branches[i]->name, branch_name)) {
            return repository->branches[i];
        }
    }
    // if the target branch was not found.
    fprintf(stderr, "target branch \'%s\' not found.\n", branch_name);
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
    // find which branch we are going to switch to.
    branch_t* target = 0x0;
    size_t target_idx = 0;

    // find the branch in the repositories branches.
    for (size_t i = 0; i < repository->count; i++) {
        if (!strcmp(repository->branches[i]->name, name)) {
            target = repository->branches[i];
            target_idx = i;
            break;
        }
    }

    // if we did not find the target branch, exit(-1).
    if (!target) {
        fprintf(stderr,"strcmp; branch does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // if we are already on the branch, do nothing.
    if (repository->idx == target_idx) {
        printf("already on branch \'%s\'\n", name);
        exit(0);
    }

    // find the common ancestor of the branch via timestamp, then
    //  do a checkout or rollback based on the direction that we are
    //  heading with this commit.
    branch_t* current = get_active_branch_repository(repository);
    commit_t* ancestor = find_common_ancestor(current , target);
    if (!ancestor) {
        // this should NOT happen, warn the user.
        printf("warning; no ancestor commit was found (branch is unrelated).\n");
        if (current->count > 0) {
            // rollback to the first commit.
            rollback(current, current->commits[0]);
            if (current->count > 0)
                apply_inverse_commit(current->commits[0]);
        }

        // checkout target branch to its head.
        if (target->count > 0) {
            // start from clean slate and apply all commits.
            for (size_t i = 0; i <= target->idx && i < target->count; i++)
                apply_forward_commit(target->commits[i]);
        }
    }
    else {
        // do a switch, rollback to the ancestor, and then checkout the target commit.
        printf("ancestor commit found; %s\n", strtrm(strsha1(ancestor->hash), 15));

        // rollback to the ancestor commit...
        long ancestor_idx = find_index_commit(current, ancestor);
        if (ancestor_idx >= 0 && ancestor_idx < current->idx)
            for (size_t i = current->idx; i > ancestor_idx; i--)
                apply_inverse_commit(current->commits[i]);

        // checkout to the ancestor commit, then to the target head.
        long head_idx = find_index_commit(target, ancestor);
        if (head_idx >= 0) {
            // apply forward commits from the ancestor to the head.
            for (long i = head_idx + 1; i <= (long) target->idx && i < (long) target->count; i++)
                apply_forward_commit(target->commits[i]);
        }
    }

    // update the repository.
    repository->idx = target_idx;
    write_repository(repository);
};