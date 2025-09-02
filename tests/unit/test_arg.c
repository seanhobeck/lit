/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file test_arg.c
 *    testing the src/arg.c functions, responsible for parsing command-line
 *      arguments and returning a struct containing the parsed arguments.
 */
#include "../tapi.h"

/*! @uses puts */
#include <stdio.h>

/*! @uses calloc, free */
#include <stdlib.h>

/*! @uses testing against this header and the code provided. */
#include "../../src/arg.h"

/// @test test the parse_args function with no arguments.
e_tapi_test_result_t
test_parse_args_no_args() {
    // arrange.
    char* argv[] = { "lit" };
    int argc = 1;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_NONE, \
        "parse_args set type to something other than NONE with no args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(capture->data != 0x0, "parse_args didn't print to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with init.
e_tapi_test_result_t
test_parse_args_init() {
    // arrange.
    char* argv[] = { "lit", "-i" };
    int argc = 2;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_INIT, \
        "parse_args set type to something other than INIT given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with help.
e_tapi_test_result_t
test_parse_args_help() {
    // arrange.
    char* argv[] = { "lit", "-h" };
    int argc = 2;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_HELP, \
        "parse_args set type to something other than HELP given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(capture->data != 0x0, "parse_args didn't print to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with version.
e_tapi_test_result_t
test_parse_args_version() {
    // arrange.
    char* argv[] = { "lit", "-v" };
    int argc = 2;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_VERSION, \
        "parse_args set type to something other than VERSION given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(capture->data != 0x0, "parse_args didn't print to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with commit (including a message).
e_tapi_test_result_t
test_parse_args_commit_message() {
    // arrange.
    char* argv[] = { "lit", "-c", "this is an example commit message" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_COMMIT, \
        "parse_args set type to something other than COMMIT given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with rollback (including a hash).
e_tapi_test_result_t
test_parse_args_rollback_hash() {
    // arrange.
    char* argv[] = { "lit", "-r", "ec1e7fb8656dba32737acabc2e5a1fb2d02a973f" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_ROLLBACK, \
        "parse_args set type to something other than ROLLBACK given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with checkout (including a hash).
e_tapi_test_result_t
test_parse_args_checkout_hash() {
    // arrange.
    char* argv[] = { "lit", "-C", "ec1e7fb8656dba32737acabc2e5a1fb2d02a973f" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_CHECKOUT, \
        "parse_args set type to something other than CHECKOUT given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with status.
e_tapi_test_result_t
test_parse_args_status() {
    // arrange.
    char* argv[] = { "lit", "-s" };
    int argc = 2;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_STATUS, \
        "parse_args set type to something other than STATUS given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with add inode.
e_tapi_test_result_t
test_parse_args_add_inode() {
    // arrange.
    char* argv[] = { "lit", "-a", "something.txt" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_ADD_INODE, \
        "parse_args set type to something other than ADD_INODE given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with delete inode.
e_tapi_test_result_t
test_parse_args_delete_inode() {
    // arrange.
    char* argv[] = { "lit", "-d", "something.txt" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_DELETE_INODE, \
        "parse_args set type to something other than DELETE_INODE given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with modify inode.
e_tapi_test_result_t
test_parse_args_modify_inode() {
    // arrange.
    char* argv[] = { "lit", "-m", "something.txt" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_MODIFIED_INODE, \
        "parse_args set type to something other than MODIFIED_INODE given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with add branch
e_tapi_test_result_t
test_parse_args_add_branch() {
    // arrange.
    char* argv[] = { "lit", "-aB", "dev1" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_CREATE_BRANCH, \
        "parse_args set type to something other than CREATE_BRANCH given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with delete branch
e_tapi_test_result_t
test_parse_args_delete_branch() {
    // arrange.
    char* argv[] = { "lit", "-dB", "dev1" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_DELETE_BRANCH, \
        "parse_args set type to something other than DELETE_BRANCH given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with switch branch
e_tapi_test_result_t
test_parse_args_switch_branch() {
    // arrange.
    char* argv[] = { "lit", "-sB", "dev1" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_SWITCH_BRANCH, \
        "parse_args set type to something other than SWITCH_BRANCH given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with rebase branch
e_tapi_test_result_t
test_parse_args_rebase_branch() {
    // arrange.
    char* argv[] = { "lit", "-rB", "origin", "dev1" };
    int argc = 4;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_REBASE_BRANCH, \
        "parse_args set type to something other than REBASE_BRANCH given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(args.argv[3] == argv[3], \
        "parse_args did not set argv[3] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with clear cache
e_tapi_test_result_t
test_parse_args_clear_cache() {
    // arrange.
    char* argv[] = { "lit", "-cc", };
    int argc = 2;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_CLEAR_CACHE, \
        "parse_args set type to something other than CLEAR_CACHE given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with add tag given a hash and name.
e_tapi_test_result_t
test_parse_args_add_tag_hash() {
    // arrange.
    char* argv[] = { "lit", "-aT",
        "ec1e7fb8656dba32737acabc2e5a1fb2d02a973f",
        "rebase_window10" };
    int argc = 4;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_ADD_TAG, \
        "parse_args set type to something other than ADD_TAG given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(args.argv[3] == argv[3], \
        "parse_args did not set argv[3] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
    free(capture->data);
    free(capture);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the parse_args function with delete tag given a name.
e_tapi_test_result_t
test_parse_args_delete_tag_name() {
    // arrange.
    char* argv[] = { "lit", "-dT", "rebase_window10" };
    int argc = 3;
    arg_t args = {};

    // act.
    tapi_output_capture_t* capture = tapi_capture_output(stdout);
    parse_args(argc, argv, &args);
    tapi_stop_capture_output(capture, stdout);

    // assert
    tapi_assert(args.type == E_ARG_TYPE_DELETE_TAG, \
        "parse_args set type to something other than DELETE_TAG given args.");
    tapi_assert(args.argv[0] == argv[0], \
        "parse_args did not set argv[0] correctly.");
    tapi_assert(args.argv[1] == argv[1], \
        "parse_args did not set argv[1] correctly.");
    tapi_assert(args.argv[2] == argv[2], \
        "parse_args did not set argv[2] correctly.");
    tapi_assert(capture->data == 0x0, "parse_args printed to stdout.");
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
            .name = "test_parse_args_no_args",
            .test = test_parse_args_no_args,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_init",
            .test = test_parse_args_init,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_help",
            .test = test_parse_args_help,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_version",
            .test = test_parse_args_version,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_commit_message",
            .test = test_parse_args_commit_message,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_rollback_hash",
            .test = test_parse_args_rollback_hash,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_checkout_hash",
            .test = test_parse_args_checkout_hash,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_add_inode",
            .test = test_parse_args_add_inode,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_delete_inode",
            .test = test_parse_args_delete_inode,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_modify_inode",
            .test = test_parse_args_modify_inode,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_add_branch",
            .test = test_parse_args_add_branch,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_delete_branch",
            .test = test_parse_args_delete_branch,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_switch_branch",
            .test = test_parse_args_switch_branch,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_rebase_branch",
            .test = test_parse_args_rebase_branch,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_clear_cache",
            .test = test_parse_args_clear_cache,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_add_tag_hash",
            .test = test_parse_args_add_tag_hash,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_parse_args_delete_tag_name",
            .test = test_parse_args_delete_tag_name,
            .setup = 0x0, .teardown = 0x0,
        },
    };

    // run the tests.
    tapi_run(tests, 17);
    return 0;
};