/**
* @author Sean Hobeck
 * @date 2025-07-22
 *
 * @file repo.c
 *    the repository module in the version control system, it is responsible for handling branches,
 *    and their locations, as well as the overall state of the repository.
 */
#include <repo.h>

/*! @uses mkdir, getcwd. */
#include <sys/types.h>
#include <sys/stat.h>

/*! @uses printf, fprintf, perror, fopen, fclose, fscanf. */
#include <stdio.h>

/*! @uses strcpy, strncpy. */
#include <string.h>

/*! @uses malloc, free. */
#include <stdlib.h>

/**
 * @brief create a new repository with the given main branch name.
 */
int repository_create() {
    // create the '.lit' directory in the current working directory.
    if (mkdir(".lit", 0755) == -1) {
        fprintf(stderr, "mkdir failed; '.lit' directory already exists.\n");
        return -1;
    }

    // create a new repository structure.
    repository_t repo = {
        .main = branch_create("main"), // create the main branch.
        .n_branches = 0u, // no extra branches yet.
        .branches = 0x0, // no extra branches yet.
    };
    sprintf(repo.main.path, ".lit/main/"); // set the main branch path.

    // write the repository to disk.
    repository_write(&repo);
    return 0;
};


/**
 * @brief write the repository to disk in our '.lit' directory.
 */
void repository_write(const repository_t* repo) {
    // open the file '.lit/repository' for writing.
    FILE* f = fopen(".lit/repository.txt", "w");
    if (!f) {
        perror("fopen failed; could not open repository file for writing.\n");
        return;
    }

    // write the main branch information.
    fprintf(f, "main_branch:%s\n", repo->main.name);
    fprintf(f, "branch_count:%lu\n", repo->n_branches);
    for (unsigned long i = 0u; i < repo->n_branches; i++) {
        fprintf(f, "branch:%lu:%s\n", i, repo->branches[i].name);
    };
    fclose(f);
};

/**
 * @brief read the repository in our cwd.
 *
 * @return a repository_t structure containing the repository information.
 */
repository_t repository_read() {
    // create a temporary repository structure.
    repository_t repo = {
        .main = {0},
        .n_branches = 0u,
        .branches = 0x0,
    };
    // open the file '.lit/repository' for reading.
    FILE* f = fopen(".lit/repository.txt", "r");
    if (!f) {
        perror("fopen failed; could not open repository file for reading.\n");
        return repo;
    }

    // read the main branch information.
    char main_branch[128u];
    int scanned = fscanf(f, "main_branch:%127[^\n]\n", main_branch);
    if (scanned != 1) {
        perror("fscanf failed; could not read main branch name.\n");
        fclose(f);
        return repo;
    }
    strncpy(repo.main.name, main_branch, sizeof(repo.main.name) - 1);
    strncpy(repo.main.path, ".lit/main/", sizeof(repo.main.path) - 1);

    // read the number of branches.
    unsigned long branch_count = 0u;
    scanned = fscanf(f, "branch_count:%lu\n", &branch_count);
    if (scanned != 1) {
        perror("fscanf failed; could not read branch count.\n");
        fclose(f);
        return repo;
    }
    // if there are no branches, return the repository.
    if (branch_count == 0u) {
        repo.n_branches = 0u;
        repo.branches = 0x0;
        fclose(f);
        return repo;
    }

    // write the branches to the repository structure.
    repo.n_branches = branch_count;
    repo.branches = malloc(sizeof(branch_t) * branch_count);
    if (!repo.branches) {
        perror("malloc failed; could not allocate memory for branches.\n");
        fclose(f);
        return repo;
    }
    for (unsigned long i = 0u; i < branch_count; i++) {
        char branch_name[128u];
        scanned = fscanf(f, "branch:%lu:%127[^\n]\n", &i, branch_name);
        if (scanned != 2) {
            perror("fscanf failed; could not read branch name.\n");
            fclose(f);
            free(repo.branches);
            return repo;
        }

        /// we need to actually make a structure that contains the data of the branch,
        /// and not allocate empty bytes to write something, else this will fail.
        repo.branches[i] = branch_read(branch_name);
    };
    return repo;
};