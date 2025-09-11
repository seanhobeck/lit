/**
 * @author Sean Hobeck
 * @date 2025-09-10
 *
 * @file test_cli.c
 *    testing the src/cli.c functions, responsible for handling the command-line
 *      interface and performing operations on the repository.
 */
#include "../tapi.h"

/*! @uses puts */
#include <stdio.h>

/*! @uses calloc, free */
#include <stdlib.h>

/*! @uses stat, struct stat */
#include <sys/stat.h>

/*! @uses strcmp */
#include <string.h>

/*! @uses bool, true, false */
#include <stdbool.h>

/*! @uses testing against this header and the code provided. */
#include "../../src/cli.h"

/// @note a overall teardown function to remove the .lit directory.
void
teardown_repo() {
    // remove the .lit directory and all subdirectories.
    system("rm -rf .lit/");
    system("rm -f example.c");
};

/// @note a setup function to create a .lit directory before each test.
void
setup_repo() {
    // copy the directory to the wd and the file.
    system("cp -r data/workspace_rebase/.lit/ .lit/");
    system("cp data/workspace_rebase/example.c example.c");
};

/// @test test the handle_init function specifically from cli_handle.
e_tapi_test_result_t
test_handle_init() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_INIT,
        .argv = (char*[]) { "lit", "-i" },
        .argc = 2,
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_init failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_init didn't print to stdout.");

    // check that the .lit directory was created as well as the subdirs.
    struct stat st;
    tapi_assert(stat(".lit/", &st) == 0, \
        "cli_handle failed to create the \'.lit/\' directory.");
    tapi_assert(stat(".lit/objects", &st) == 0, \
        "cli_handle failed to create the \'.lit/objects/\' directory.");
    tapi_assert(stat(".lit/objects/commits", &st) == 0, \
        "cli_handle failed to create the \'.lit/objects/commits\' directory.");
    tapi_assert(stat(".lit/objects/diffs", &st) == 0, \
        "cli_handle failed to create the \'.lit/objects/diffs\' directory.");
    tapi_assert(stat(".lit/refs", &st) == 0, \
        "cli_handle failed to create the \'.lit/refs/\' directory.");
    tapi_assert(stat(".lit/refs/heads", &st) == 0, \
        "cli_handle failed to create the \'.lit/refs/heads/\' directory.");
    tapi_assert(stat(".lit/refs/tags/", &st) == 0, \
        "cli_handle failed to create the \'.lit/refs/\' directory.");

    // check that the repository file was created as well as the origin branch.
    FILE* fp = fopen(".lit/refs/heads/origin", "r");
    tapi_assert(fp != 0x0, "cli_handle failed to create the \'.lit/refs/heads/origin\' file.");
    fclose(fp);
    fp = fopen(".lit/index", "r");
    tapi_assert(fp != 0x0, "cli_handle failed to create the \'.lit/index\' file.");
    fclose(fp);
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_status function specifically from cli_handle.
e_tapi_test_result_t
test_handle_status() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_STATUS,
        .argv = (char*[]) { "lit", "-s" },
        .argc = 2
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_status failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_status didn't print to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_commit funcction specifically from cli_handle
///     (without shelved changes).
e_tapi_test_result_t
test_handle_commit_no_shelved_changes() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_COMMIT,
        .argv = (char*[]) { "lit", "-c", "example.txt" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stderr);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stderr);

    // assert.
    tapi_assert(result == -1, "result from cli_handle did not return -1.\n");
    tapi_assert(capture->data != 0x0, "handle_commit didn't print to stderr.");
    tapi_assert(strcmp(capture->data, "no diffs to commit; nothing stashed.\n") == 0x0, \
        "handle_commit didn't print the correct error message to stderr.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @note setup function to also have shelved changes.
void
setup_repo_shelved() {
    // copy the directory to the wd and the file.
    system("cp -r data/workspace_basic_shelved/.lit/ .lit/");
    system("cp data/workspace_basic_shelved/example.c example.c");
};

/// @test test the handle_commit function specifically from cli_handle
///     (with shelved changes, thus it should commit).
e_tapi_test_result_t
test_handle_commit_shelved_changes() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_COMMIT,
        .argv = (char*[]) { "lit", "-c", "printing argc*2 in example.c" },
        .argc = 3
    };
    // open the file of the origin branch and check the count.
    FILE* fptr = fopen(".lit/refs/heads/origin", "r");
    if (!fptr) return E_TAPI_TEST_RESULT_SKIP;
    fclose(fptr);

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_commit failed / returned -1");
    tapi_assert(capture->data != 0x0, "handle_commit didn't print to stdout.");
    tapi_assert(strcmp(capture->data, \
        "added commit 'printing argc*2 in example.c' to branch 'origin' with 1 change(s).\n") == 0, \
        "stdout message does not match, printed something else to stdout.");

    // we then need to check if the file was actually commited.
    struct stat st;
    tapi_assert(stat(".lit/objects/shelved/origin", &st) != 0, \
        "handle_commit failed to remove the shelved directory.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @note setup function to be on a commit behind of the head on origin.
void
setup_repo_checkout() {
    // copy the directory to the wd and the file.
    system("cp -r data/workspace_commit_behind/.lit/ .lit/");
    system("cp data/workspace_commit_behind/example.c example.c");
};

/// @test test the handle_cr_move function specifically from cli_handle.
e_tapi_test_result_t
test_handle_checkout() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_CHECKOUT,
        .argv = (char*[]) { "lit", "-C", \
            "d26a53beca2dfb2f06af973a34b3b88ff86c6866" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(capture->data != 0x0, "handle_cr_move didn't print to stdout.");

    // read the branch file.
    FILE* fptr = fopen(".lit/refs/heads/origin", "r");
    tapi_assert(fptr != 0x0, "handle_cr_move failed to open the branch file for reading.");
    char line[256];
    for (size_t i = 1; i <= 3; i++) {
        if (!fgets(line, 256, fptr)) {
            tapi_assert(false, "handle_cr_move failed to read the branch file.");
        }
    }
    size_t index = 0;
    int scanned = sscanf(line, "idx:%lu\n", &index);
    tapi_assert(scanned == 1, "handle_cr_move failed to read the index file.");
    tapi_assert(index == 1, "handle_cr_move failed to change the active commit index.");

    // close everything.
    fclose(fptr);
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_cr_move function specifically from cli_handle.
e_tapi_test_result_t
test_handle_rollback() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_ROLLBACK,
        .argv = (char*[]) { "lit", "-r", \
            "b2b20989de7ff7ea33071ad8efb376f8dcb936bc" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_cr_move didn't return 0.");
    tapi_assert(capture->data != 0x0, "handle_cr_move didn't print to stdout.");

    // read the branch file.
    FILE* fptr = fopen(".lit/refs/heads/origin", "r");
    tapi_assert(fptr != 0x0, "handle_cr_move failed to open the branch file for reading.");
    char line[256];
    for (size_t i = 1; i <= 3; i++) {
        if (!fgets(line, 256, fptr)) {
            tapi_assert(false, "handle_cr_move failed to read the branch file.");
        }
    }
    size_t index = 0;
    int scanned = sscanf(line, "idx:%lu\n", &index);
    tapi_assert(scanned == 1, "handle_cr_move failed to read the index file.");
    tapi_assert(index == 0, "handle_cr_move failed to change the active commit index.");

    // close everything.
    fclose(fptr);
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_add_delete_inode function specifically from cli_handle.
e_tapi_test_result_t
test_handle_add_inode() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_ADD_INODE,
        .argv = (char*[]) { "lit", "-a", "newfile.txt" },
        .argc = 3
    };
    system("echo 'this is a new file' > newfile.txt");

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_add_inode failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_add_inode didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/objects/shelved/origin/", &st) == 0, \
        "cli_handle failed to create the shelved directory.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_add_delete_inode function specifically from cli_handle.
e_tapi_test_result_t
test_handle_delete_inode() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_DELETE_INODE,
        .argv = (char*[]) { "lit", "-d", "newfile.txt" },
        .argc = 3,
    };
    system("echo 'this is a new file' > newfile.txt");

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_add_inode failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_add_inode didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/objects/shelved/origin/", &st) == 0, \
        "cli_handle failed to create the shelved directory.");
    tapi_assert(stat("newfile.txt", &st) != 0x0, \
        "handle_delete_inode failed to delete the file from the working directory.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @note setup function to have a file that has already been added to the history.
void
setup_repo_modified() {
    // copy the directory to the wd and the file.
    system("cp -r data/workspace_modified/.lit/ .lit/");
    system("cp data/workspace_modified/example.c example.c");
};

/// @test test the handle_modified_inode function specifically from cli_handle.
e_tapi_test_result_t
test_handle_modified_inode() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_MODIFIED_INODE,
        .argv = (char*[]) { "lit", "-m", "example.c" },
        .argc = 3,
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_modified_inode failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_modified_inode didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/objects/shelved/origin/", &st) == 0, \
        "cli_handle failed to create the shelved directory.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_create_branch function specifically from cli_handle.
e_tapi_test_result_t
test_handle_create_branch() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_CREATE_BRANCH,
        .argv = (char*[]) { "lit", "-aB", "new-branch" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_create_branch failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_create_branch didn't print to stdout.");
    tapi_assert(strcmp(capture->data, "created branch \'new-branch\' from \'origin\'.\n") == 0x0, \
        "handle_create_branch did not print the correct message to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/refs/heads/new-branch", &st) == 0, \
        "handle_create_branch failed to create the new branch.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_delete_branch function specifically from cli_handle.
e_tapi_test_result_t
test_handle_delete_branch() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_DELETE_BRANCH,
        .argv = (char*[]) { "lit", "-dB", "dev" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_delete_branch failed to return 0.");
    tapi_assert(capture->data != 0x0, "handle_delete_branch didn't print to stdout.");
    tapi_assert(strcmp(capture->data, "deleted branch \'dev\'.\n") == 0x0, \
        "handle_delete_branch did not print the correct message to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/refs/heads/dev", &st) != 0, \
        "handle_delete_branch failed to delete the new branch.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_switch_branch function specifically from cli_handle.
e_tapi_test_result_t
test_handle_switch_branch() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_SWITCH_BRANCH,
        .argv = (char*[]) { "lit", "-sB", "dev" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_switch_branch failed to return 0.");
    struct stat st;
    tapi_assert(stat(".lit/refs/heads/dev", &st) == 0x0, \
        "handle_switch_branch modified the \'.lit/refs/heads/dev\' branch.");
    FILE* fptr = fopen(".lit/index", "r");
    tapi_assert(fptr != 0x0, "handle_switch_branch failed to open the file.");
    char line[256];
    fgets(line, 256, fptr);
    fclose(fptr);
    tapi_assert(strcmp(line, "active:1\n") == 0x0, \
        "handle_switch_branch didn't switch the branch in \'.lit/index\'");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_rebase_branch function specifically from cli_handle.
e_tapi_test_result_t
test_handle_rebase_branch() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_REBASE_BRANCH,
        .argv = (char*[]) { "lit", "-rB", "origin", "dev" },
        .argc = 4
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_rebase_branch failed to return 0.");
    tapi_assert(strcmp(capture->data, "successfully rebased \'dev\' onto \'origin\' with 1 commit(s).\n") == 0x0,
        "handle_rebase_branch didn't print the correct message to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/refs/heads/origin", &st) == 0x0, \
        "handle_rebase_branch modified a \'.lit/refs/heads/origin\'.");
    tapi_assert(stat(".lit/refs/heads/dev", &st) == 0x0, \
        "handle_rebase_branch modified a \'.lit/refs/heads/dev\'.");
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_clear_cache function specifically from cli_handle.
e_tapi_test_result_t
test_handle_clear_cache() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_CLEAR_CACHE,
        .argv = (char*[]) { "lit", "-cc" },
        .argc = 2
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_clear_cache failed to clear the cache.");
    tapi_assert(capture->data != 0x0, "handle_clear_cache didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/objects/commits/ab/cdefgh", &st) != 0, \
        "handle_clear_cache failed to delete useless caches.");
    tapi_assert(stat(".lit/objects/diffs/ab/cdefgh", &st) != 0, \
        "handle_clear_cache failed to delete useless caches.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the handle_add_tag function specifically from cli_handle.
e_tapi_test_result_t
test_handle_add_tag() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_ADD_TAG,
        .argv = (char*[]) { "lit", "-aT", \
            "d26a53beca2dfb2f06af973a34b3b88ff86c6866", \
            "rebase_window" },
        .argc = 4
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_add_tag failed to return 0.")
    tapi_assert(capture->data != 0x0, "handle_add_tag didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/refs/tags/rebase_window", &st) == 0, \
        "handle_add_tag failed to create tag file in repository.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @note setup function to have a pre-existing tag called 'rebase_window'.
void
setup_repo_tag() {
    // copy the directory to the wd and the file.
    system("cp -r data/workspace_basic_tag/.lit/ .lit/");
    system("cp data/workspace_basic_tag/example.c example.c");
};

/// @test test the handle_delete_tag function specifically from cli_handle.
e_tapi_test_result_t
test_handle_delete_tag() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_DELETE_TAG,
        .argv = (char*[]) { "lit", "-dT", "rebase_window" },
        .argc = 3
    };

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    int result = cli_handle(args);
    tapi_stop_capture_output(capture, stdout);

    // assert.
    tapi_assert(result == 0, "handle_delete_tag failed to return 0.")
    tapi_assert(capture->data != 0x0, "handle_delete_tag didn't print to stdout.");
    struct stat st;
    tapi_assert(stat(".lit/refs/tags/rebase_window", &st) != 0, \
        "handle_add_tag failed to delete tag file in repository.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

int main() {
    // initialize the testing api.
    tapi_init();

    // add our tests here.
    tapi_test_t tests[] = {
        {
            .name = "test_handle_init",
            .test = test_handle_init,
            .setup = 0x0, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_status",
            .test = test_handle_status,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_commit_no_shelved_changes",
            .test = test_handle_commit_no_shelved_changes,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_commit_shelved_changes",
            .test = test_handle_commit_shelved_changes,
            .setup = setup_repo_shelved, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_checkout",
            .test = test_handle_checkout,
            .setup = setup_repo_checkout, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_rollback",
            .test = test_handle_rollback,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_add_inode",
            .test = test_handle_add_inode,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_delete_inode",
            .test = test_handle_delete_inode,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_modified_inode",
            .test = test_handle_modified_inode,
            .setup = setup_repo_modified, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_create_branch",
            .test = test_handle_create_branch,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_delete_branch",
            .test = test_handle_delete_branch,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_switch_branch",
            .test = test_handle_switch_branch,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_rebase_branch",
            .test = test_handle_rebase_branch,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_clear_cache",
            .test = test_handle_clear_cache,
            .setup = setup_repo_shelved, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_add_tag",
            .test = test_handle_add_tag,
            .setup = setup_repo, .teardown = teardown_repo,
        },
        {
            .name = "test_handle_delete_tag",
            .test = test_handle_delete_tag,
            .setup = setup_repo_tag, .teardown = teardown_repo,
        }
    };

    // run the tests.
    tapi_run(tests, 16);
    return 0;
};