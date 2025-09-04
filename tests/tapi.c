/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file tapi.c
 *    the testing api header, responsible for tests and testing functions.
 */
#include "tapi.h"

/*! @uses va_start, va_arg, va_end */
#include <stdarg.h>

/*! @uses assert */
#include <assert.h>

/*! @uses clock_t, clock */
#include <time.h>

/*! @uses strcmp */
#include <string.h>

/*! @uses dup, dup2 */
#include <unistd.h>

/// @note file pointers for logging and error logging.
FILE* fptr_log;
char log_path[512];

/**
 * @brief initialize the testing api, setting up log files and stdout logging.
 */
void
tapi_init() {
    // get the current time.
    time_t now = time(0x0);

    // create the log file path.
    snprintf(log_path, 512, "test_%ld.log", now);

    // open both log files.
    fptr_log = fopen(log_path, "w");
    if (!fptr_log) {
        fprintf(stderr, "fopen failed; could not open log files for writing.\n");
        exit(EXIT_FAILURE);
    }
    tapi_log(E_TAPI_LOG_LEVEL_INFO, "(tapi) initialized.\n");
};

/**
 * @brief run the tests provided in the array of tapi_test_t.
 *
 * @param tests the array of tests to be run.
 * @param n the number of tests in the array.
 */
void
tapi_run(tapi_test_t* tests, size_t n) {
    // sanity assert.
    assert(tests != 0x0);

    // iterate through each test and run it.
    size_t count = 0;
    clock_t start = clock();
    for (size_t i = 0; i < n; i++) {
        // run setup if required.
        if (tests[i].setup) tests[i].setup();

        // log the start of the test.
        tapi_log(E_TAPI_LOG_LEVEL_WARN, "running test '%s'...\n", tests[i].name);

        // run the test and get the result.
        e_tapi_test_result_t result = tests[i].test();

        // log the end of the test.
        tapi_log(E_TAPI_LOG_LEVEL_INFO, "test '%s' %s.\n",
            tests[i].name,
            result == E_TAPI_TEST_RESULT_PASS ? "passed" : \
            result == E_TAPI_TEST_RESULT_FAIL ? "failed" : "skipped");

        // increment count if passed.
        if (result == E_TAPI_TEST_RESULT_PASS) count++;

        // run teardown if required.
        if (tests[i].teardown) tests[i].teardown();
    }

    // log the final results.
    clock_t end = clock();
    tapi_log(E_TAPI_LOG_LEVEL_INFO, \
        "(tapi) finished testing with %d/%d test(s) passing in %.3f ms.\n",
        (int) count, (int) n, ((double) (end - start) / CLOCKS_PER_SEC * 1000.0f));

    // if all the tests passed, remove the log file.
    fclose(fptr_log);
    if (count == n)
        remove(log_path);
};

/**
 * @brief log the formatted string to stdout and a respective log file.
 *
 * @param level the log level of the message.
 * @param fmt the format string.
 * @param ... virtual/packed arguments and parameters.
 */
void
tapi_log(e_tapi_log_level_t level, const char* fmt, ...) {
    // get the correct FILE* to write to.
    FILE* stream = (level == E_TAPI_LOG_LEVEL_ERROR ? stderr : stdout);

    // push the respective color to the respective FILE*
    if (level == E_TAPI_LOG_LEVEL_WARN)
        fprintf(stream, "[\x1b[1;33m");
    else if (level == E_TAPI_LOG_LEVEL_ERROR)
        fprintf(stream, "[\x1b[1;31m");
    else
        fprintf(stream, "[\x1b[1;32m");

    // create the actual formatted string.
    char* string = calloc(1, 512);
    va_list args;
    va_start(args, fmt);
    vsnprintf(string, 512, fmt, args);
    va_end(args);

    // print the formatted string to the respective FILE* (stdout or stderr).
    fprintf(stream, "%s\x1b[0m]: %s",
        level == E_TAPI_LOG_LEVEL_INFO ? "info" :
        level == E_TAPI_LOG_LEVEL_WARN ? "warn" : "error",
        string);
    fprintf(fptr_log, "[%s]: %s\n",
        level == E_TAPI_LOG_LEVEL_INFO ? "info" :
        level == E_TAPI_LOG_LEVEL_WARN ? "warn" : "error",
        string);
    free(string);
};

/**
 * @brief start capturing stdout and stderr output.
 *
 * @param stream the stream to capture (stdout or stderr).
 * @return the tapi_output_capture_t structure to hold the captured data.
 */
tapi_output_capture_t*
tapi_capture_output(FILE* stream) {
    // allocate the structure.
    tapi_output_capture_t* capture = calloc(1, sizeof *capture);

    // open the streams.
    int fds[2];
    if (pipe(fds) == -1) {
        fprintf(stderr, "pipe failed; could not create pipe for stdout and stderr.\n");
        exit(EXIT_FAILURE);
    }
    capture->pipe_rd = fds[0];
    capture->pipe_wr = fds[1];

    // flush stdout.
    fflush(stream);
    capture->saved_fd = dup(fileno(stream));
    if (capture->saved_fd == -1) {
        fprintf(stderr, "pipe failed; could not create pipe for stdout and stderr.\n");
        exit(EXIT_FAILURE);
    }
    if (dup2(capture->pipe_wr, fileno(stream)) == -1) {
        fprintf(stderr, "dup2 failed; could not copy over pipe_wr fd to stdout.\n");
        exit(EXIT_FAILURE);
    }

    // stdout is now the pipe write end, close the copy
    close(capture->pipe_wr);
    capture->pipe_wr = -1;
    setvbuf(stream, 0x0, _IONBF, 0);
    return capture;
};

/**
 * @brief stop capturing stdout and stderr output.
 *
 * @param capture the tapi_output_capture_t structure holding the captured data.
 * @param stream the stream to stop capturing (stdout or stderr).
 */
void
tapi_stop_capture_output(tapi_output_capture_t* capture, FILE* stream) {
    // flush both stdout stream.
    fflush(stream);
    if (dup2(capture->saved_fd, fileno(stream)) == -1) {
        fprintf(stderr, "dup2 failed; could not copy saved fd over to stdout.\n");
        exit(EXIT_FAILURE);
    }
    close(capture->saved_fd);
    capture->saved_fd = -1;

    // read all data from pipe.
    char buf[4096];
    size_t capacity = 0, len = 0;
    capture->data = 0x0;
    capture->size = 0;

    // close writer side so pipe will hit EOF
    ssize_t n;
    while ((n = read(capture->pipe_rd, buf, 4096)) > 0) {
        // if len is greater we realloc the string.
        if (len + n > capacity) {
            capacity = capacity ? capacity * 2 : 8192;

            // calculate size, plus null terminator.
            if (capacity < len + n + 1) capacity = len + n + 1;
            capture->data = realloc(capture->data, capacity);
            if (!capture->data) {
                fprintf(stderr, "realloc failed; could not allocate memory for stdout capture.\n");
                close(capture->pipe_rd);
                exit(EXIT_FAILURE);
            }
        }
        memcpy(capture->data + len, buf, n);
        len += n;
    }
    close(capture->pipe_rd);
    capture->pipe_rd = -1;
    if (n == -1) {
        fprintf(stderr, "read failed; could not read from stdout pipe.\n");
        exit(EXIT_FAILURE);
    }

    // terminate the string/data.
    if (capture->data) capture->data[len] = '\0';
    capture->size = len;
};