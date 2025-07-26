/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file main.c
 *    entry point for the program, we read arguments and process files
 *    and diffs from there.
 */
/*! @uses printf. */
#include <stdio.h>

/*! @uses diff_create, diff_write, diff_read. */
#include <diff.h>

/*! @uses repo_create, repository_read, repository_write. */
#include <repo.h>

/*! @uses branch_create, branch_write, branch_read, branch_add_commit. */
#include <branch.h>

#include "roll.h"

int main(int argc, char *argv[]) {
    // diff_t original = diff_file_new("ex/old.v");
    // diff_t write = diff_file_modified("ex/old.v","ex/new.v");
    // diff_write(&write, "ex/new.diff");
    // remove("ex/old.v");
    //
    // repository_create();
    // repository_t repo = repository_read();
    //
    // commit_t commit = commit_create("initial commit", repo.main.name);
    // commit_add_diff(&commit, &original);
    //
    // branch_add_commit(&commit, &repo.main.history);
    //
    // commit_t commit2 = commit_create("second commit", repo.main.name);
    // commit_add_diff(&commit2, &write);
    //
    // branch_add_commit(&commit2, &repo.main.history);
    //
    // repo.main.current_commit = 1u;
    // branch_write(&repo.main);

    // branch_t branch = branch_read("main");
    //
    // printf("number of commits in main branch: %lu\n", branch.history.n_commits);
    // for (unsigned long i = 0u; i < branch.history.n_commits; i++) {
    //     commit_t* c = branch.history.commits[i];
    //     printf("commit %lu: %s, %s, %s\n", i, c->message, c->timestamp, strsha1(c->hash));
    //     for (unsigned long j = 0u; j < c->n_diffs; j++) {
    //         diff_t* d = c->diffs[j];
    //         printf("diff %lu: type: %s, name: %s\n", j, diff_type_str(d->type), d->n_name);
    //     }
    // }
    //
    // rollback(&branch, branch.history.commits[0]);

    diff_t diff = diff_folder("ex/something/");
    return 0;
};