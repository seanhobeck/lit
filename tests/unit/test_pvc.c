/**
 * @author Sean Hobeck
 * @date 2025-08-31
 *
 * @file test_pvc.c
 *    testing the src/pvc.c functions, responsible for collecting filenames into
 *      a vectorized structure for diffing and snapshotting purposes.
 */
#include "../tapi.h"

/*! @uses puts */
#include <stdio.h>

/*! @uses calloc, free */
#include <stdlib.h>

/*! @uses testing against this header and the code provided. */
#include "../../src/pvc.h"

/// @test test the vector_push function on empty nodes.
e_tapi_test_result_t
test_vector_push_empty_nodes() {
    // arrange.
    vector_t* vector = calloc(1, sizeof *vector);
    *vector = (vector_t) {
        .nodes = 0x0,
        .count = 0,
        .cap = 0
    };
    vinode_t* node = calloc(1, sizeof *node);
    *node = (vinode_t) {
        .path = "test_path",
        .name = "test_name",
        .mtime = 0,
        .type = E_PVC_INODE_TYPE_FILE
    };

    // act.
    vector_push(vector, node);

    // assert.
    tapi_assert(vector != 0x0, "vector_push failed to allocate vector.");
    tapi_assert(vector->nodes != 0x0, "vector_push failed to allocate nodes.");
    tapi_assert(vector->count != 0, "vector_push failed to increment count.");
    tapi_assert(vector->cap == 16, "vector_push failed to set initial capacity.");
    for (size_t i = 0; i < vector->count; i++)
        free(vector->nodes[i]);
    free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test the vector_push function on some given nodes.
e_tapi_test_result_t
test_vector_push_given_nodes() {
    // arrange.
    vector_t* vector = calloc(1, sizeof *vector);
    *vector = (vector_t) {
        .nodes = calloc(16, sizeof *vector->nodes),
        .count = 3,
        .cap = 16
    };
    vinode_t* node1 = calloc(1, sizeof *node1),
        * node2 = calloc(1, sizeof *node2),
        * node3 = calloc(1, sizeof *node3),
        * node4 = calloc(1, sizeof *node4);
    *node1 = (vinode_t) {
        .path = "test_path1",
        .name = "test_name1",
        .mtime = 0,
        .type = E_PVC_INODE_TYPE_FILE
    };
    vector->nodes[0] = node1;
    *node2 = (vinode_t) {
        .path = "test_path2",
        .name = "test_name2",
        .mtime = 0,
        .type = E_PVC_INODE_TYPE_FOLDER
    };
    vector->nodes[1] = node2;
    *node3 = (vinode_t) {
        .path = "test_path3",
        .name = "test_name3",
        .mtime = 0,
        .type = E_PVC_INODE_TYPE_FILE
    };
    vector->nodes[2] = node3;
    *node4 = (vinode_t) {
        .path = "test_path4",
        .name = "test_name4",
        .mtime = 0,
        .type = E_PVC_INODE_TYPE_FOLDER
    };

    // act.
    vector_push(vector, node4);

    // assert.
    tapi_assert(vector != 0x0, "vector_push failed to allocate vector.");
    tapi_assert(vector->nodes != 0x0, "vector_push failed to allocate nodes.");
    tapi_assert(vector->count == 4, "vector_push failed to increment count.");
    tapi_assert(vector->cap == 16, "vector_push failed to set initial capacity.");
    for (size_t i = 0; i < vector->count; i++)
        free(vector->nodes[i]);
    free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

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
    vector_t* vector = vector_collect(path, E_PVC_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 3, "vector_collect returned incorrect number of files.");
    vector_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with no recursion and only one subitem.
e_tapi_test_result_t
test_vector_collect_no_recurse_one_subitem() {
    // arrange.
    const char* path = "./test_data/no_recurse/subdir/";

    // act.
    vector_t* vector = vector_collect(path, E_PVC_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 1, "vector_collect returned incorrect number of files.");
    vector_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with no recursion and no subitems.
e_tapi_test_result_t
test_vector_collect_no_recurse_no_subitems() {
    // arrange.
    const char* path = "./test_data/recurse/";

    // act.
    vector_t* vector = vector_collect(path, E_PVC_TYPE_NO_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 0, "vector_collect returned incorrect number of files.");
    vector_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion.
e_tapi_test_result_t
test_vector_collect_recurse() {
    // arrange.
    const char* path = "./test_data/";

    // act.
    vector_t* vector = vector_collect(path, E_PVC_TYPE_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 6, "vector_collect returned incorrect number of files.");
    vector_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion with one subitem.
e_tapi_test_result_t
test_vector_collect_recurse_one_subitem() {
    // arrange.
    const char* path = "./test_data/no_recurse/subdir/";

    // act.
    vector_t* vector = vector_collect(path, E_PVC_TYPE_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 1, "vector_collect returned incorrect number of files.");
    vector_free(vector);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the vector_collect function with recursion and no subitems.
e_tapi_test_result_t
test_vector_collect_recurse_no_subitems() {
    // arrange.
    const char* path = "./test_data/recurse/";

    // act.
    vector_t* vector = vector_collect(path, E_PVC_TYPE_RECURSE);

    // assert.
    tapi_assert(vector != 0x0, "vector_collect failed to return a vector.");
    tapi_assert(vector->count == 0, "vector_collect returned incorrect number of files.");
    vector_free(vector);
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
        {
            .name = "test_vector_push_empty_nodes",
            .test = test_vector_push_empty_nodes,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_vector_push_given_nodes",
            .test = test_vector_push_given_nodes,
            .setup = 0x0, .teardown = 0x0,
        },
    };

    // run the tests.
    tapi_run(tests, 8);
    return 0;
};