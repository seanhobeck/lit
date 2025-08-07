/**
* @author Sean Hobeck
 * @date 2025-07-29
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

/**
 * @brief create a new repository with the given main branch name.
 *
 * @return a 0 if successful in creating the directory, or -1 if .lit directory already exists.
 */
repository_t* repository_create() {
    // create the '.lit' directory in the current working directory.
    if (mkdir(".lit", 0777) == -1) {
        fprintf(stderr, "mkdir failed; '.lit' directory already exists.\n");
        exit(-1);
    }

    // create a new repository structure.
    repository_t* repo = calloc(1, sizeof *repo);
    repo->main = branch_create("main");
    repo->count = 0;
    repo->capacity = 0;
    repo->idx = 0;
    repo->branches = 0x0;

    // write the repository and branch to disk.
    branch_write(repo->main);
    repository_write(repo);
    return repo;
};

/**
 * @brief write the repository to disk in our '.lit' directory.
 *
 * @param repo the repository_t structure to be written to a file.
 */
void repository_write(const repository_t* repo) {
    // open the file '.lit/repository' for writing.
    FILE* f = fopen(".lit/repository", "w");
    if (!f) {
        perror("fopen failed; could not open repository file for writing.\n");
        return;
    }

    // write the main branch information.
    fprintf(f, "main_branch:%s\n", repo->main->name);
    fprintf(f, "current_branch:%lu\n", repo->idx);
    fprintf(f, "branch_count:%lu\n", repo->count);
    for (size_t i = 0u; i < repo->count; i++)
        fprintf(f, "branch:%lu:%s\n", i, repo->branches[i]->name);
    fclose(f);
};

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t* repository_read() {
    // create a temporary repository structure.
    repository_t* repo = calloc(1, sizeof *repo);

    // open the file '.lit/repository' for reading.
    FILE* f = fopen(".lit/repository", "r");
    if (!f) {
        perror("fopen failed; could not open repository file for reading.\n");
        return 0x0;
    }

    // read the main branch information.
    char* main_branch = calloc(1, 128u);
    int scanned = fscanf(f, "main_branch:%127[^\n]\n", main_branch);
    if (scanned != 1) {
        perror("fscanf failed; could not read main branch name.\n");
        fclose(f);
        return 0x0;
    }
    repo->main = branch_read(main_branch);
    free(main_branch);

    // read the current branch index.
    size_t idx = 0u;
    scanned = fscanf(f, "current_branch:%lu\n", &idx);
    if (scanned != 1) {
        perror("fscanf failed; could not read current branch index.\n");
        fclose(f);
        return repo;
    }
    repo->idx = idx;

    // read the number of branches.
    size_t count = 0u;
    scanned = fscanf(f, "branch_count:%lu\n", &count);
    if (scanned != 1) {
        perror("fscanf failed; could not read branch count.\n");
        fclose(f);
        return repo;
    }

    // if there are no branches, return the repository.
    if (count == 0u) {
        repo->count = 0u;
        repo->branches = 0x0;
        fclose(f);
        return repo;
    }

    // write the branches to the repository structure.
    repo->count = count;
    repo->branches = calloc(1, sizeof(branch_t*) * count);
    if (!repo->branches) {
        perror("malloc failed; could not allocate memory for branches.\n");
        fclose(f);
        return repo;
    }

    // iterate through the count.
    for (size_t i = 0u; i < count; i++) {
        // allocate and read.
        char* branch_name = calloc(1, 128);
        scanned = fscanf(f, "branch:%lu:%127[^\n]\n", &i, branch_name);
        if (scanned != 2) {
            perror("fscanf failed; could not read branch name.\n");
            fclose(f);
            free(repo->branches);
            return repo;
        }

        /// we need to actually make a structure that contains the data of the branch,
        /// and not allocate empty bytes to write something, else this will fail.
        repo->branches[i] = branch_read(branch_name);
    };
    return repo;
};