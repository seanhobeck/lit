/**
 * @author Sean Hobeck
 * @date 2025-12-15
 */
#ifndef LOG_H
#define LOG_H

/**
 * enum for all the different logger levels that are being printed to the console.
 */
typedef enum {
    E_LOGGER_LEVEL_DEBUG = 0x0, /* debug information. */
    E_LOGGER_LEVEL_INFO = 0x1, /* general information. */
    E_LOGGER_LEVEL_WARNING = 0x2, /* warning the user. */
    E_LOGGER_LEVEL_ERROR = 0x3, /* fatal error. */
} e_logger_level_t;

/**
 * @brief write a log message to the stdout|sterr.
 *
 * @param level the logger level to write the message as.
 * @param format the format string to be written.
 * @param ... the variable arguments to be written.
 */
void
log(e_logger_level_t level, const char* format, ...);
#endif /* LOG_H */
