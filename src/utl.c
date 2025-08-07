/**
 * @author Sean Hobeck
 * @date 2025-07-30
 *
 * @file utl.c
 *    the utilities module, responsible for miscellaneous utility functions.
 */
#include "utl.h"

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char* strdup(const char* str) {
    // allocate memory for the string.
    char* new_str = calloc(1, strlen(str) + 1);
    if (!new_str) {
        perror("calloc failed; could not allocate memory for string.\n");
        return 0x0;
    }

    // copy the string into the new memory.
    strcpy(new_str, str);
    return new_str;
};

/**
 * @brief reduce a string to a specified length of <n>
 *
 * @param str the string to be trimmed.
 * @param n size to be trimmed at
 * @return a allocated string that has been trimmed with '...' if larger than <n>.
 */
char* strtrm(const char* str, size_t n) {
    // duplicate the string and compare length.
    char* dup = strdup(str);
    unsigned long len = strlen(dup);
    if (len <= n)
        return dup;

    // if less, allocate a new string and trim it
    //  to a length of n, ending in '...'
    char* new = calloc(1, n+1u);
    strncpy(new, dup, n);
    free(dup);
    new[n-3] = '.';
    new[n-2] = '.';
    new[n-1] = '.';
    new[n] = '\0';
    return new;
};

/**
 * @brief convert a string to a hash of <n> length.
 *
 * @param string the string to be converted into a n-length hash
 * @param n the number of characters (or bytes * 2).
 * @return a hash that is converted from the string.
 */
unsigned char* strtoha(const char* string, size_t n) {
    // iterate over n, then read each two characters from <string>
    //  into a unsigned byte, set the value in <hash> and continue.
    unsigned char* hash = calloc(1, n);
    if (!hash) {
        perror("calloc failed; could not allocate memory for hash.\n");
        exit(-1);
    }

    for (size_t i = 0; i < n; i++) {
        unsigned int byte = 0;
        if (sscanf(string + i * 2, "%2x", &byte) != 1) {
            free(hash);
            perror("sscanf failed; could not read hash.");  // or handle the error
            exit(-1);
        }
        hash[i] = (unsigned char) byte;
    }
    return hash;
};

/**
 * @brief read a file by lines.
 *
 * @param fptr the file pointer to read from.
 * @param size a pointer to an integer to store the size of the buffer.
 * @return a char** containing the lines of the file.
 */
char** freadls(FILE* fptr, size_t* size) {
    // make a maximum line buffer, and allocate the data of the file.
    char buffer[MAX_LINE_LEN];
    char** data = calloc(1, sizeof(char*) * MAX_LINES);
    size_t j = MAX_LINES;

    // iterate through each line in <fptr>.
    size_t i = 0;
    while (fgets(buffer, sizeof(buffer), fptr)) {
        // calculate line string length.
        size_t len = strlen(buffer);

        // if line is too long and doesn't end in '\n', flush the rest.
        if (len == MAX_LINE_LEN - 1 && buffer[len - 1] != '\n') {
            int ch;
            while ((ch = fgetc(fptr)) != '\n' && ch != EOF);
        }
        // strip newline.
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (i >= j) {
            // realloc if required.
            data = realloc(data, sizeof(char*) * (j + 1024ul));
            if (!data) {
                perror("realloc failed; could not allocate memory for file data.\n");
                exit(-1);
            }
        };

        // append to our data.
        data[i] = calloc(1, len);
        strncpy(data[i], buffer, len);
        i++;
    }

    // set our size reference, and return our data.
    *size = i;
    return data;
};
