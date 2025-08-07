/**
* @author Sean Hobeck
 * @date 2025-07-30
 *
 * @file utl.h
 *    the utilities module, responsible for miscellaneous utility functions.
 */
#ifndef UTL_H
#define UTL_H

// @note starting size to calculate for is 1024 lines of 256 chars per line.
#define MAX_LINES 1024ul

// @note at the most 256 characters can be stored in a line.
#define MAX_LINE_LEN 256ul

/*!
 *  @details ^^^
 *
 *  these macros above specify that you cannot have a file that exceeds 8192 lines,
 *  with 256 characters per line. this means at the MOST you will be able to handle 2 MiB files.
 */

/*! @uses FILE, fopen, fclose, fseek, etc.. */
#include <stdio.h>

/*! @uses strcpy, strcmp, strdup, etc.. */
#include <string.h>

/*! @uses malloc, free. */
#include <stdlib.h>

/*! @uses va_list, va_start, va_end. */
#include <stdarg.h>

/*! @uses size_t */
#include <stddef.h>

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char* strdup(const char* str);

/**
 * @brief reduce a string to a specified length of <n>
 *
 * @param str the string to be trimmed.
 * @param n size to be trimmed at
 * @return a allocated string that has been trimmed with '...' if larger than <n>.
 */
char* strtrm(const char* str, size_t n);

/**
 * @brief convert a string to a hash of <n> length.
 *
 * @param string the string to be converted into a n-length hash
 * @param n the number of characters (or bytes * 2).
 * @return a hash that is converted from the string.
 */
unsigned char* strtoha(const char* string, size_t n);

/**
 * @brief read a file by lines.
 *
 * @param fptr the file pointer to read from.
 * @param size a pointer to an integer to store the size of the buffer.
 * @return a char** containing the lines of the file.
 */
char** freadls(FILE* fptr, size_t* size);
#endif //UTL_H