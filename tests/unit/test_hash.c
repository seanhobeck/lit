/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file test_hash.c
 *    testing the src/hash.c functions, responsible for creating crc32, sha1,
 *      and sha256 (not anymore) hashes for files and or other data.
 */
#include "../tapi.h"

/*! @uses puts */
#include <stdio.h>

/*! @uses calloc, free */
#include <stdlib.h>

/*! @uses strcmp */
#include <string.h>

/*! @uses testing against this header and the code provided. */
#include "../../src/hash.h"

/// @test testing the sha1 function for the correct output.
e_tapi_test_result_t
test_hash_sha1_hexadecimal() {
    // arrange.
    const unsigned char data[] = { 0xde, 0xad, 0xbe, 0xef };
    const char* expected = "d78f8bb992a56a597f6c7a1fb918bb78271367eb";

    // act.
    sha1_t hash;
    sha1(data, 4, hash);
    char* str = strsha1(hash);

    // assert.
    tapi_assert(str != 0x0, "strsha1 failed to allocate string.");
    tapi_assert(strcmp(str, expected) == 0, "sha1 produced incorrect hash.");
    free(str);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the sha1 function for the correct output.
e_tapi_test_result_t
test_hash_sha1_string() {
    // arrange.
    const unsigned char data[] = "apple123";
    const char* expected = "ec1e7fb8656dba32737acabc2e5a1fb2d02a973f";

    // act.
    sha1_t hash;
    sha1(data, 8, hash);
    char* str = strsha1(hash);

    // assert.
    tapi_assert(str != 0x0, "strsha1 failed to allocate string.");
    tapi_assert(strcmp(str, expected) == 0, "sha1 produced incorrect hash.");
    free(str);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test testing the crc32 function for the correct output.
e_tapi_test_result_t
test_hash_crc32_hexadecimal() {
    // arrange.
    const unsigned char data[] = { 0xde, 0xad, 0xbe, 0xef };
    const unsigned int expected = 0x7c9ca35a;

    // act.
    ucrc32_t hash = crc32(data, 4);

    // assert.
    tapi_assert(hash == expected, "crc32 produced incorrect hash.");
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test testing the crc32 function for the correct output.
e_tapi_test_result_t
test_hash_crc32_string() {
    // arrange.
    const unsigned char data[] = "apple123";
    const unsigned int expected = 0xb91a851b;

    // act.
    ucrc32_t hash = crc32(data, 8);

    // assert.
    tapi_assert(hash == expected, "crc32 produced incorrect hash.");
    return E_TAPI_TEST_RESULT_PASS;
};

int main() {
    // initialize the testing api.
    puts("\n");
    tapi_init();

    // add our tests here.
    tapi_test_t tests[] = {
        {
            .name = "test_hash_sha1_hexadecimal",
            .test = test_hash_sha1_hexadecimal,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_hash_sha1_string",
            .test = test_hash_sha1_string,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_hash_crc32_hexadecimal",
            .test = test_hash_crc32_hexadecimal,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_hash_crc32_string",
            .test = test_hash_crc32_string,
            .setup = 0x0, .teardown = 0x0,
        },
    };
    tapi_log(E_TAPI_LOG_LEVEL_WARN,
        "skipping sha256 due to deprecation, and endian errors.\n");

    // run the tests.
    tapi_run(tests, 4);
    return 0;
};