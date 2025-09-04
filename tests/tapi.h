/**
 * @author Sean Hobeck
 * @date 2025-09-01
 *
 * @file tapi.h
 *    the testing api header, responsible for tests and testing functions.
 */
#ifndef TAPI_H
#define TAPI_H

/// @note enables POSIX 2008 functions like fdopen, fileno
#define _POSIX_C_SOURCE 200809L

/*! @uses size_t, calloc, realloc, free */
#include <stdlib.h>

/*! @uses FILE, fdopen, ... */
#include <stdio.h>

/// @note an enum for logging levels (info, warn error).
typedef enum {
    E_TAPI_LOG_LEVEL_INFO,
    E_TAPI_LOG_LEVEL_WARN,
    E_TAPI_LOG_LEVEL_ERROR
} e_tapi_log_level_t;

/// @note an enum for test results.
typedef enum {
    E_TAPI_TEST_RESULT_PASS,
    E_TAPI_TEST_RESULT_FAIL,
    E_TAPI_TEST_RESULT_SKIP
} e_tapi_test_result_t;

/// @note a function pointer type for test functions.
typedef e_tapi_test_result_t(* tapi_test_func_t)();

/// @note a function pointer type for setup and teardown functions.
typedef void(* tapi_func_t)();

/// @note a structure for a test case.
typedef struct {
    /// name, clocks to start and end, and ptrs to functions required.
    const char* name;
    tapi_test_func_t test;
    tapi_func_t setup, teardown;
} tapi_test_t;

/**
 * @brief initialize the testing api, setting up log files and stdout logging.
 */
void
tapi_init();

/**
 * @brief run the tests provided in the array of tapi_test_t.
 *
 * @param tests the array of tests to be run.
 * @param n the number of tests in the array.
 */
void
tapi_run(tapi_test_t* tests, size_t n);

/**
 * @brief log the formatted string to stdout and a respective log file.
 *
 * @param level the log level of the message.
 * @param fmt the format string.
 * @param ... virtual/packed arguments and parameters.
 */
void
tapi_log(e_tapi_log_level_t level, const char* fmt, ...);

/// @note a structure for capturing output from stdout and stderr.
typedef struct {
    /// saved dup, read end, and write end of the pipe.
    int saved_fd, pipe_rd, pipe_wr;
    char* data;
    size_t size;
} tapi_output_capture_t;

/**
 * @brief start capturing stdout and stderr output.
 *
 * @param stream the stream to capture (stdout or stderr).
 * @return the tapi_output_capture_t structure to hold the captured data.
 */
tapi_output_capture_t*
tapi_capture_output(FILE* stream);

/**
 * @brief stop capturing stdout and stderr output.
 *
 * @param capture the tapi_output_capture_t structure holding the captured data.
 * @param stream the stream to stop capturing (stdout or stderr).
 */
void
tapi_stop_capture_output(tapi_output_capture_t* capture, FILE* stream);

/// @brief an assertion macro for tests.
#define tapi_assert(condition, message) \
    if (!(condition)) { \
        tapi_log(E_TAPI_LOG_LEVEL_ERROR, "assertion failed; %s\n", message); \
        return E_TAPI_TEST_RESULT_FAIL; \
    }
#endif //TAPI_H