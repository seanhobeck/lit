/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file test_utl_io.c
 *    testing the src/utl.c functions, responsible for miscellaneous utility functions
 *      that deal with file i/o.
 */
#include "../tapi.h"

/*! @uses testing against this header and the code provided. */
#include "../../src/utl.h"

/*! @uses stat, struct stat */
#include <sys/stat.h>

/// @note setup function to create a test directory.
void
setup_existpd() {
    // create a test directory to work in.
    mkdir("test", 0777);
    mkdir("test/folder1", 0777);
    mkdir("test/folder2", 0777);
    mkdir("test/folder1/folder3", 0777);
}

/// @note teardown function to remove the test directory.
void
teardown_existpd() {
    // remove the test directory.
    remove("test/folder1/folder3/");
    remove("test/folder2/folder3/");
    remove("test/folder2/");
    remove("test/folder1");
    remove("test");
}

/// @test test that fexistpd recognizes an existing path.
e_tapi_test_result_t
test_fexistpd_existing_path() {
    // arrange.
    const char* path = "test/folder1/";

    // act.
    const int result = fexistpd(path);

    // assert.
    tapi_assert(result != -1, "fexistpd failed to recognize existing path.");
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test test that fexistpd recognizes an non-existing path.
e_tapi_test_result_t
test_fexistpd_non_existing_path() {
    // arrange.
    const char* path = "test/folder2/folder3/";

    // act.
    const int result = fexistpd(path);

    // assert.
    tapi_assert(result != -1, "fexistpd failed to recognize a non-existing path.");
    struct stat st;
    tapi_assert(stat(path, &st) == 0, "fexistpd failed to create the non-existing path.");
    return E_TAPI_TEST_RESULT_PASS;
}

/// @note setup function to write to a test file.
void
setup_fwritels() {
    // create a dummy file to write to.
    fclose(fopen("testfile.txt", "w+"));
}

/// @note teardown function to remove the test file.
void
teardown_fwritels() {
    // remove the dummy file.
    remove("testfile.txt");
}

/// @test test that fwritels writes lines to a file.
e_tapi_test_result_t
test_fwritels_standard_file() {
    // arrange.
    const char* path = "testfile.txt";
    char* lines[] = {
        "line 1",
        "line 2",
        "line 3"
    };
    const size_t n = 3;

    // act.
    fwritels(path, lines, n);

    // assert.
    FILE* f = fopen(path, "r");
    tapi_assert(f != 0x0, "fwritels failed to open the file for reading.");
    size_t i = 0;
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, MAX_LINE_LEN, f) != 0x0) {
        // strip newline.
        const size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        // compare lines.
        tapi_assert(strcmp(buffer, lines[i]) == 0, "fwritels failed to write the correct lines to the file.");
        i++;
    }
    tapi_assert(i == n, "fwritels failed to write the correct number of lines to the file.");
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test that fwritels nothing to a file.
e_tapi_test_result_t
test_fwritels_empty_lines() {
    // arrange.
    const char* path = "testfile.txt";
    char* lines[] = {};
    const size_t n = 0;

    // act.
    fwritels(path, lines, n);

    // assert.
    FILE* f = fopen(path, "r");
    tapi_assert(f != 0x0, "fwritels failed to open the file for reading.");
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, MAX_LINE_LEN, f) != 0x0) {
        tapi_assert(buffer == 0, "fwritels wrote a non-empty line to the file.");
    }
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test that fcleanls cleans lines correctly.
e_tapi_test_result_t
test_fcleanls_standard_lines() {
    // arrange.
    char *lines [] = {
        " line 1",
        "+ line 2",
        "- line 3",
        " line 4"
    };
    char *expected[] = {
        "line 1",
        "line 2",
        "line 4",
    };
    const size_t n = 4;
    size_t k = 0;

    // act.
    char** cleaned = fcleanls(lines, n, &k);

    // assert.
    tapi_assert(k == n - 1, "fcleanls failed to clean the lines correctly.");
    for (size_t i = 0; i < k; i++) {
        tapi_assert(strcmp(cleaned[i], expected[i]) == 0,
                    "fcleanls failed to clean the lines correctly.");
    }
    free(cleaned);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test that fcleanls cleans lines correctly (partially).
e_tapi_test_result_t
test_fcleanls_partial_lines() {
    // arrange.
    char *lines [] = {
        " line 1",
        "+ line 2",
        "- line 3",
        " line 4"
    }, *expected[] = {
        "line 1",
        "line 2",
    };
    const size_t n = 3;
    size_t k = 0;

    // act.
    char** cleaned = fcleanls(lines, n, &k);

    // assert.
    tapi_assert(k == n - 1, "fcleanls failed to clean the lines correctly.");
    for (size_t i = 0; i < k; i++)
        tapi_assert(strcmp(cleaned[i], expected[i]) == 0, \
            "fcleanls failed to clean the lines correctly.");
    free(cleaned);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test that fcleanls cleans lines correctly (no removes).
e_tapi_test_result_t
test_fcleanls_no_removes() {
    // arrange.
    char *lines [] = {
        " line 1",
        "+ line 2",
        "+ line 3",
        " line 4"
    }, *expected[] = {
        "line 1",
        "line 2",
        "line 3",
        "line 4"
    };
    const size_t n = 4;
    size_t k = 0;

    // act.
    char** cleaned = fcleanls(lines, n, &k);

    // assert.
    tapi_assert(k == n, "fcleanls failed to clean the lines correctly.");
    for (size_t i = 0; i < k; i++)
        tapi_assert(strcmp(cleaned[i], expected[i]) == 0, \
            "fcleanls failed to clean the lines correctly.");
    free(cleaned);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @test test that fcleanls cleans lines correctly (no removes & partial).
e_tapi_test_result_t
test_fcleanls_no_removes_partial() {
    // arrange.
    char *lines [] = {
        " line 1",
        "+ line 2",
        "+ line 3",
        " line 4"
    }, *expected[] = {
        "line 1",
        "line 2",
    };
    const size_t n = 2;
    size_t k = 0;

    // act.
    char** cleaned = fcleanls(lines, n, &k);

    // assert.
    tapi_assert(k == n, "fcleanls failed to clean the lines correctly.");
    for (size_t i = 0; i < k; i++)
        tapi_assert(strcmp(cleaned[i], expected[i]) == 0, \
            "fcleanls failed to clean the lines correctly.");
    free(cleaned);
    return E_TAPI_TEST_RESULT_PASS;
}

/// @note setup function to read lines from a test file.
void
setup_freadls_full() {
    FILE* f = fopen("testfile.txt", "w+");
    fprintf(f, " line 1\n");
    fprintf(f, "+ line 2\n");
    fprintf(f, "- line 3\n");
    fprintf(f, "line 4\n");
    fclose(f);
}

/// @note setup function to read lines from an empty test file.
void
setup_freadls_empty() {
    FILE* f = fopen("testfile.txt", "w+");
    fprintf(f, "");
    fclose(f);
}

/// @note teardown function to remove the test file.
void
teardown_freadls() {
    // remove the dummy file.
    remove("testfile.txt");
};

/// @test test that freadls reads lines from a file.
e_tapi_test_result_t
test_freadls_standard_file() {
    // arrange.
    const char* path = "testfile.txt";
    size_t k = 0;

    // act.
    char** lines = freadls(fopen(path, "r"), &k);

    // assert.
    tapi_assert(k == 4, "freadls failed to read all the lines from the file.");
    tapi_assert(lines[0] != 0x0, "freadls failed to read the first line from the file.");
    tapi_assert(strcmp(lines[0], " line 1") == 0x0, "freadls read the incorrect data from the file.");
    tapi_assert(lines[1] != 0x0, "freadls failed to read the first line from the file.");
    tapi_assert(strcmp(lines[1], "+ line 2") == 0x0, "ffreadls read the incorrect data from the file.");
    tapi_assert(lines[2] != 0x0, "freadls failed to read the first line from the file.");
    tapi_assert(strcmp(lines[2], "- line 3") == 0x0, "freadls read the incorrect data from the file.");
    tapi_assert(lines[3] != 0x0, "freadls failed to read the first line from the file.");
    tapi_assert(strcmp(lines[3], "line 4") == 0x0, "freadls read the incorrect data from the file.");
    free(lines);
    return E_TAPI_TEST_RESULT_PASS;
};

/// @test test that freadls reads no lines from an empty file.
e_tapi_test_result_t
test_freadls_empty_file() {
    // arrange.
    const char* path = "testfile.txt";
    size_t k = 0;

    // act.
    char** lines = freadls(fopen(path, "r"), &k);
    free(lines);

    // assert.
    tapi_assert(k == 0, "freadls read lines from an empty file.");
    return E_TAPI_TEST_RESULT_PASS;
};

int main() {
    // initialize the testing api.
    puts("\n");
    tapi_init();

    // add our tests here.
    tapi_test_t tests[] = {
        {
            .name = "test_fexistpd_existing_path",
            .test = test_fexistpd_existing_path,
            .setup = setup_existpd, .teardown = teardown_existpd,
        },
        {
            .name = "test_fexistpd_non_existing_path",
            .test = test_fexistpd_non_existing_path,
            .setup = setup_existpd, .teardown = teardown_existpd,
        },
        {
            .name = "test_fwritels_standard_file",
            .test = test_fwritels_standard_file,
            .setup = setup_fwritels, .teardown = teardown_fwritels,
        },
        {
            .name = "test_fcleanls_standard_lines",
            .test = test_fcleanls_standard_lines,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_fcleanls_partial_lines",
            .test = test_fcleanls_partial_lines,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_fcleanls_no_removes",
            .test = test_fcleanls_no_removes,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_fcleanls_no_removes_partial",
            .test = test_fcleanls_no_removes_partial,
            .setup = 0x0, .teardown = 0x0,
        },
        {
            .name = "test_fwritels_empty_lines",
            .test = test_fwritels_empty_lines,
            .setup = setup_fwritels, .teardown = teardown_fwritels,
        },
        {
            .name = "test_freadls_standard_file",
            .test = test_freadls_standard_file,
            .setup = setup_freadls_full, .teardown = teardown_freadls,
        },
        {
            .name = "test_freadls_empty_file",
            .test = test_freadls_empty_file,
            .setup = setup_freadls_empty, .teardown = teardown_freadls,
        },
    };

    // run the tests.
    tapi_run(tests, 10);
    return 0;
}
