/**
 * @author Sean Hobeck
 * @date 2025-09-01
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
    system("cp -r workspace/.lit/ .lit/");
    system("cp workspace/example.c example.c");
};

/// @test test the handle_init function specifically from cli_handle.
e_tapi_test_result_t
test_handle_init() {
    // arrange.
    arg_t args = {
        .type = E_ARG_TYPE_INIT,
        .argv = (char*[]) { "lit", "-i" }
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
        .argv = (char*[]) { "lit", "-s" }
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
    };

    // run the tests.
    tapi_run(tests, 2);
    return 0;
};