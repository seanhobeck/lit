/**
 * @author Sean Hobeck
 * @date 2025-11-13
 *
 * @file test_inw.c
 *    testing the src/inw.c functions, responsible for collecting filenames into
 *      a vectorized structure for diffing and snapshotting purposes.
 */
#include "../tapi.h"

/*! @uses puts */
#include <stdio.h>

/*! @uses calloc, free */
#include <stdlib.h>

/*! @uses testing against this header and the code provided. */
#include "../../src/inw.h"

/// @note a setup function to create test data.
void
setup_vector_collect() {
    // create the test_data directory.
    system("mkdir -p ./test_data/no_recurse");
    system("touch ./test_data/no_recurse/file1.txt");
    system("touch ./test_data/no_recurse/file2.txt");
    system("mkdir -p ./test_data/no_recurse/subdir");
    system("mkdir -p ./test_data/recurse");
    system("touch ./test_data/no_recurse/subdir/file3.txt");
}

/// @note a teardown function to remove test data.
void
teardown_vector_collect() {
    // remove the test_data directory.
    system("rm -rf ./test_data");
}

/// @test testing the vector_collect function with no recursion.
e_tapi_test_result_t
test_vector_collect_no_recurse() {
    // arrange.
    const char* path = "./test_data/no_recurse/";

    // act.
    dyna_t* array = inw_walk(path, E_INW_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(array != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(array->length == 3, "vector_collect returned incorrect number of files.");
    dyna_free(array);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with no recursion and only one subitem.
e_tapi_test_result_t
test_vector_collect_no_recurse_one_subitem() {
    // arrange.
    const char* path = "./test_data/no_recurse/subdir/";

    // act.
    dyna_t* array = inw_walk(path, E_INW_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(array != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(array->length == 1, "vector_collect returned incorrect number of files.");
    dyna_free(array);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with no recursion and no subitems.
e_tapi_test_result_t
test_vector_collect_no_recurse_no_subitems() {
    // arrange.
    const char* path = "./test_data/recurse/";

    // act.
    dyna_t* array = inw_walk(path, E_INW_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(array != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(array->length == 0, "vector_collect returned incorrect number of files.");
    dyna_free(array);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion.
e_tapi_test_result_t
test_vector_collect_recurse() {
    // arrange.
    const char* path = "./test_data/";

    // act.
    dyna_t* array = inw_walk(path, E_INW_TYPE_RECURSE);

    // assert.
    tapi_assert(array != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(array->length == 6, "vector_collect returned incorrect number of files.");
    dyna_free(array);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion with one subitem.
e_tapi_test_result_t
test_vector_collect_recurse_one_subitem() {
    // arrange.
    const char* path = "./test_data/no_recurse/subdir/";

    // act.
    dyna_t* vector = inw_walk(path, E_INW_TYPE_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->length == 1, "vector_collect returned incorrect number of files.");
    dyna_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion and no subitems.
e_tapi_test_result_t
test_vector_collect_recurse_no_subitems() {
    // arrange.
    const char* path = "./test_data/recurse/";

    // act.
    dyna_t* vector = inw_walk(path, E_INW_TYPE_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->length == 0, "vector_collect returned incorrect number of files.");
    dyna_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

int main() {
    // initialize the testing api.
    puts("\n");
    tapi_init();

    // add our tests here.
    tapi_test_t tests[] = {
        {
            .name = "test_vector_collect_no_recurse",
            .test = test_vector_collect_no_recurse,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
        {
            .name = "test_vector_collect_no_recurse_one_subitem",
            .test = test_vector_collect_no_recurse_one_subitem,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
        {
            .name = "test_vector_collect_no_recurse_no_subitems",
            .test = test_vector_collect_no_recurse_no_subitems,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
        {
            .name = "test_vector_collect_recurse",
            .test = test_vector_collect_recurse,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
        {
            .name = "test_vector_collect_recurse_one_subitem",
            .test = test_vector_collect_recurse_one_subitem,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
        {
            .name = "test_vector_collect_recurse_no_subitems",
            .test = test_vector_collect_recurse_no_subitems,
            .setup = setup_vector_collect, .teardown = teardown_vector_collect,
        },
    };

    // run the tests.
    tapi_run(tests, 6);
    return 0;
};