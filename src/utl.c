/**
 * @author Sean Hobeck
 * @date 2025-08-12
 *
 * @file utl.c
 *    the utilities module, responsible for miscellaneous utility functions.
 */
#include "utl.h"

/*! @uses errno */
#include <errno.h>

/*! @uses mkdir */
#include <sys/stat.h>

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char*
strdup(const char* str) {
    // allocate memory for the string.
    char* new_str = calloc(1, strlen(str) + 1);
    if (!new_str) {
        fprintf(stderr,"calloc failed; could not allocate memory for string.\n");
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
char*
strtrm(const char* str, size_t n) {
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
 * @param str the string to be converted into a n-length hash
 * @param n the number of characters (or bytes * 2).
 * @return a hash that is converted from the string.
 */
unsigned char*
strtoha(const char* str, size_t n) {
    // iterate over n, then read each two characters from <string>
    //  into a unsigned byte, set the value in <hash> and continue.
    unsigned char* hash = calloc(1, n);
    if (!hash) {
        fprintf(stderr,"calloc failed; could not allocate memory for hash.\n");
        exit(-1);
    }

    for (size_t i = 0; i < n; i++) {
        unsigned int byte = 0;
        if (sscanf(str + i * 2, "%2x", &byte) != 1) {
            free(hash);
            fprintf(stderr,"sscanf failed; could not read hash.");  // or handle the error
            exit(-1);
        }
        hash[i] = (unsigned char) byte;
    }
    return hash;
};

/**
 * @brief check if a folder path absolutely exists in the filesystem.
 *
 * @param path the absolute path of the location that we are trying to check exists.
 * @return 0 if the parent directories already exist, and -1 if it doesnt.
 */
int
fexistpd(const char* path) {
    // string duplicate the path to avoid modifying the original.
    char *dpath = strdup(path);
    if (!dpath) {
        fprintf(stderr,"strdup failed; could not duplicate path.\n");
        exit(-1);
    }

    // iterate through each character in the path.
    for (char *p = dpath + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            // check if the directory exists, if not return -1.
            if (mkdir(dpath, MKDIR_MOWNER) != 0 && errno != EEXIST) {
                free(dpath);
                return -1;
            }
            *p = '/';
        }
    }
    free(dpath);
    return 0;
};

/**
 * @brief write lines out to a file at <path>.
 *
 * @param path the path of the file to write the lines to.
 * @param lines the array of lines that are to be written.
 * @param n the count/capacity of ptr <lines> as an array.
 */
void
fwritels(const char* path, char** lines, size_t n) {
    // ensure that the parent directories exist on <path>.
    if (fexistpd(path) == -1) {
        fprintf(stderr,"fexistpd failed; could not create parent directories.\n");
        exit(-1); // exit on failure.
    }

    // open the file.
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr,"fopen failed; could not open file for rollback writing.\n");
        exit(-1);
    }

    // iterate over the lines and use fprintf();
    for (size_t i = 0; i < n; i++)
        fprintf(f, "%s\n", lines[i]);
    fclose(f);
};

/**
 * @brief given some lines from a lcs algorithm, clean them and return
 *  only the lines without the '+' and '-'.
 *
 * @param lines the array of strings to be cleaned.
 * @param n the number of lines to be cleaned.
 * @param k a pointer to a value for the amount of lines cleaned.
 * @return the new array of strings that are cleaned.
 */
char**
fcleanls(char** lines, size_t n, size_t* k) {
    // allocate the cleaned lines
    char** clines = calloc(1, n * sizeof(char*));

    // iterate...
    size_t m = 0;
    for (size_t i = 0; i < n; i++) {
        char* line = lines[i];

        // if there is a space, there isn't a two char prefix.
        if (line[0] == ' ') clines[m++] = strdup(line + 1);
        else if (line[0] == '+') clines[m++] = strdup(line + 2);
        else if (line[0] != '-') clines[m++] = strdup(line);
    }
    *k = m;
    return clines;
};

/**
 * @brief read a file by lines.
 *
 * @param fptr the file pointer to read from.
 * @param size a pointer to an integer to store the size of the buffer.
 * @return a char** containing the lines of the file.
 */
char**
freadls(FILE* fptr, size_t* size) {
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
                fprintf(stderr,"realloc failed; could not allocate memory for file data.\n");
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

/**
 * @brief read parent path given some path.
 *
 * @param path the full path provided.
 * @return the allocated parent path.
 */
char*
rpwd(const char* path) {
    // given some empty path, return 0x0.
    if (!path) return 0x0;

    // find the last slash ptr in <copy>, make sure it is not
    //   the end of <copy>, and then return the duped string.
    char* copy = strdup(path);
    char* last_slash_ptr = strrchr(copy, '/');
    if (last_slash_ptr && last_slash_ptr != copy) {
        *last_slash_ptr = 0x0;
        return copy;
    }
    return 0x0;
}