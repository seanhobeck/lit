/**
 * @author Sean Hobeck
 * @date 2025-12-15
 */
#include "log.h"

/*! @uses va_list, va_start, va_end.. */
#include <stdarg.h>

/*! @uses vfprintf, fprintf, FILE*. */
#include <stdio.h>

/**
 * @brief write a log message to the stdout|sterr.
 *
 * @param level the logger level to write the message as.
 * @param format the format string to be written.
 * @param ... the variable arguments to be written.
 */
void
llog(e_logger_level_t level, const char* format, ...) {
    /* gather the variable argument to be written. */
    va_list args;
    va_start(args, format);

    /* find the correct stream and print 'fatal' if it is an error. */
    FILE* stream = (level == E_LOGGER_LEVEL_ERROR ? stderr : stdout);
    if (stream == stderr) {
        fprintf(stream, "fatal: ");
    }

    /* print and end. */
    vfprintf(stream, format, args);
    va_end(args);
};