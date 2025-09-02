/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file test_utl_str.c
 *    testing the src/utl.c functions, responsible for miscellaneous utility functions
 *      that deal with strings & the path method.
 */
#include "../tapi.h"

/*! @uses testing against this header and the code provided. */
#include "../../src/utl.h"

/// @test testing the strdup function with a standard string.
e_tapi_test_result_t
test_strdup_standard_string() {
    // arrange.
    const char* original = "Hello, World!";
    char* duplicated = 0x0;

    // act.
    duplicated = strdup(original);

    // assert.
    tapi_assert(strcmp(original, duplicated) == 0x0, "strings are not equal.");
    tapi_assert(duplicated != 0x0, "strdup returned null.");
    free(duplicated);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the strdup function with an empty string.
e_tapi_test_result_t
test_strdup_empty_string() {
    // arrange.
    const char* original = "";
    char* duplicated = 0x0;

    // act.
    duplicated = strdup(original);

    // assert.
    tapi_assert(strcmp(original, duplicated) == 0x0, "strings are not equal.");
    tapi_assert(duplicated != 0x0, "strdup returned null.");
    free(duplicated);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strdup function with a \0 string.
e_tapi_test_result_t
test_strdup_null_string() {
    // arrange.
    const char* original = "\0";
    char* duplicated = 0x0;

    // act.
    duplicated = strdup(original);

    // assert.
    tapi_assert(strcmp(original, duplicated) == 0x0, "strings are not equal.");
    tapi_assert(duplicated != 0x0, "strdup returned null.");
    free(duplicated);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtrm function with a standard string.
e_tapi_test_result_t
test_strtrm_standard_string() {
    // arrange.
    const char* original = "Hello, World!", *expected = "He...";
    size_t n = 5;

    // act.
    char* result = strtrm(original, n);

    // assert.
    tapi_assert(strcmp(result, expected) == 0x0, "trimmed string does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtrm function with a longer string.
e_tapi_test_result_t
test_strtrm_longer_string() {
    // arrange.
    const char* original = "Hello, World! This is some very interesting text"
        "that I have written here!", *expected = "Hello, World! Thi...";
    size_t n = 20;

    // act.
    char* result = strtrm(original, n);

    // assert.
    tapi_assert(strcmp(result, expected) == 0x0, "trimmed string does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtrm function with an empty string.
e_tapi_test_result_t
test_strtrm_empty_string() {
    // arrange.
    const char* original = "", *expected = "";
    size_t n = 0;

    // act.
    char* result = strtrm(original, n);

    // assert.
    tapi_assert(strcmp(result, expected) == 0x0, "trimmed string does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtrm function with an n value greater than the length of the string.
e_tapi_test_result_t
test_strtrm_n_greater_than_length() {
    // arrange.
    const char* original = "", *expected = "";
    size_t n = 10;

    // act.
    char* result = strtrm(original, n);

    // assert.
    tapi_assert(strcmp(result, expected) == 0x0, "trimmed string does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtoha function with a standard string.
e_tapi_test_result_t
test_strtoha_standard_string() {
    // arrange.
    const char* string = "21df0bde4db3ef";
    unsigned char expected[] = { 0x21, 0xdf, 0x0b, 0xde, 0x4d, 0xb3, 0xef };
    size_t n = 7;

    // act.
    unsigned char* result = strtoha(string, n);

    // assert.
    for (size_t i = 0; i < n; i++)
        tapi_assert(result[i] == expected[i], "hash byte does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtoha function with a standard string with odd length.
e_tapi_test_result_t
test_strtoha_odd_length() {
    // arrange.
    const char* string = "21df0bde4db3efa";
    unsigned char expected[] = { 0x21, 0xdf, 0x0b, 0xde, 0x4d, 0xb3, 0xef };
    size_t n = 7;

    // act.
    unsigned char* result = strtoha(string, n);

    // assert.
    for (size_t i = 0; i < n; i++)
        tapi_assert(result[i] == expected[i], "hash byte does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the strtoha function with an empty string.
e_tapi_test_result_t
test_strtoha_empty_string() {
    // arrange.
    const char* string = "";
    size_t n = 0;

    // act.
    unsigned char* result = strtoha(string, n);

    // assert.
    tapi_assert(result == 0x0, "returned something other than null.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the rpwd function with a standard path.
e_tapi_test_result_t
test_rpwd_standard_path() {
    // arrange.
    const char* path = "/home/user/documents/file.txt";
    const char* expected = "/home/user/documents";

    // act.
    char* result = rpwd(path);

    // assert.
    tapi_assert(strcmp(result, expected) == 0x0, "returned path does not match expected.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the rpwd function with a single slash.
e_tapi_test_result_t
test_rpwd_single_slash() {
    // arrange.
    const char* path = "/";

    // act.
    char* result = rpwd(path);

    // assert.
    tapi_assert(result == 0x0, "returned path is something other than null.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the rpwd function with a empty path.
e_tapi_test_result_t
test_rpwd_empty_path() {
    // arrange.
    const char* path = "";

    // act.
    char* result = rpwd(path);

    // assert.
    tapi_assert(result == 0x0, "returned path is something other than null.");
    free(result);
    return E_TAPI_TEST_RESULT_PASS;
}

int
main() {
    // initialize the testing api.
    puts("\n");
    tapi_init();

    // add our tests here.
    tapi_test_t tests[] = {
        {
            .name = "test_strdup_standard_string",
            .test = test_strdup_standard_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strdup_empty_string",
            .test = test_strdup_empty_string,
            .setup = 0x0, .teardown = 0x0,
        },{
            .name = "test_strdup_null_string",
            .test = test_strdup_null_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtrm_standard_string",
            .test = test_strtrm_standard_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtrm_longer_string",
            .test = test_strtrm_longer_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtrm_empty_string",
            .test = test_strtrm_empty_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtrm_n_greater_than_length",
            .test = test_strtrm_n_greater_than_length,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtoha_standard_string",
            .test = test_strtoha_standard_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtoha_odd_length",
            .test = test_strtoha_odd_length,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_strtoha_empty_string",
            .test = test_strtoha_empty_string,
            .setup = 0x0, .teardown = 0x0,
        },
            {
            .name = "test_rpwd_standard_path",
            .test = test_rpwd_standard_path,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_rpwd_single_slash",
            .test = test_rpwd_single_slash,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_rpwd_empty_path",
            .test = test_rpwd_empty_path,
            .setup = 0x0, .teardown = 0x0,
        },
    };

    // run the tests.
    tapi_run(tests, 13);
    return 0;
}
