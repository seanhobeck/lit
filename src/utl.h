/**
 * @author Sean Hobeck
 * @date 2025-08-31
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

// @note owner(user) only read/write permissions on a folder.
#define MKDIR_MOWNER 0755

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

/*! @uses; assert */
#include <assert.h>

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char*
strdup(const char* str);

/**
 * @brief reduce a string to a specified length of <n>
 *
 * @param str the string to be trimmed.
 * @param n size to be trimmed at
 * @return a allocated string that has been trimmed with '...' if larger than <n>.
 */
char*
strtrm(const char* str, size_t n);

/**
 * @brief convert a string to a hash of <n> length.
 *
 * @param str the string to be converted into a n-length hash
 * @param n the number of characters (or bytes * 2).
 * @return a hash that is converted from the string.
 */
unsigned char*
strtoha(const char* str, size_t n);

/**
 * @brief check if a folder path absolutely exists in the filesystem.
 *
 * @param path the absolute path of the location that we are trying to check exists.
 * @return 0 if the parent directories already exist, and -1 if it doesnt.
 */
int
fexistpd(const char* path);

/**
 * @brief write lines out to a file at <path>.
 *
 * @param path the path of the file to write the lines to.
 * @param lines the array of lines that are to be written.
 * @param n the count/capacity of ptr <lines> as an array.
 */
void
fwritels(const char* path, char** lines, size_t n);

/**
 * @brief given some lines from a lcs algorithm, clean them and return
 *  only the lines without the '+' and ignoring '-'.
 *
 * @param lines the array of strings to be cleaned.
 * @param n the number of lines to be cleaned.
 * @param k a pointer to a value for the amount of lines cleaned.
 * @return the new array of strings that are cleaned.
 */
char**
fcleanls(char** lines, size_t n, size_t* k);

/**
 * @brief read a file by lines.
 *
 * @param fptr the file pointer to read from.
 * @param size a pointer to an integer to store the size of the buffer.
 * @return a char** containing the lines of the file.
 */
char**
freadls(FILE* fptr, size_t* size);

/**
 * @brief read parent path given some path.
 *
 * @param path the full path provided.
 * @return the allocated parent path.
 */
char*
rpwd(const char* path);
#endif //UTL_H