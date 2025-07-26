/**
 * @author Sean Hobeck
 * @date 2025-07-24
 *
 * @file diff.c
 *    the diff module, responsible for creating comparisons between files and folders,
 *    while also maintaining a history of changes in '.diff' files.
 */
#include <diff.h>

/*! @uses FILE, fopen, fclose, fseek, etc.. */
#include <stdio.h>

/*! @uses strcpy, strcmp, strdup, etc.. */
#include <string.h>

/*! @uses malloc, free. */
#include <stdlib.h>

/*! @uses va_list, va_start, va_end. */
#include <stdarg.h>

/*! @uses rollback_to_diff */
#include <roll.h>

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char* strdup(const char* str) {
    // allocate memory for the string.
    char* new_str = malloc(strlen(str) + 1);
    if (!new_str) {
        perror("malloc failed; could not allocate memory for string.\n");
        return 0x0;
    }

    // copy the string into the new memory.
    strcpy(new_str, str);
    return new_str;
};

/**
 * @brief read a file into a buffer.
 *
 * @param fptr the file pointer to read from.
 * @param size a pointer to an integer to store the size of the buffer.
 * @return a char** containing the lines of the file.
 */
char** rindat(FILE* fptr, int* size) {
    char buffer[MAX_LINE_LEN];
    char** data = malloc(sizeof(char*) * MAX_LINES);
    int i = 0;

    while (fgets(buffer, sizeof(buffer), fptr)) {
        size_t len = strlen(buffer);

        // if line is too long and doesn't end in '\n', flush the rest
        if (len == MAX_LINE_LEN - 1 && buffer[len - 1] != '\n') {
            int ch;
            while ((ch = fgetc(fptr)) != '\n' && ch != EOF);
        }

        // strip newline
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (i >= MAX_LINES) break;
        data[i++] = strdup(buffer);
    }

    *size = i;
    return data;
};

/**
 * @brief calculate the longest common subsequence (lcs) between two strings.
 *
 * @param a the first string.
 * @param m the length of the first string.
 * @param b the second string.
 * @param n the length of the second string.
 * @param diff the diff_t structure to store the differences.
 */
void lcs(char** a, const int m, char** b, const int n, diff_t* diff) {
    // @ref[https://en.wikipedia.org/wiki/Longest_common_subsequence]
    int **dp = malloc((m+1) * sizeof(int*));
    for (int i = 0; i <= m; i++)
        dp[i] = calloc(n+1, sizeof(int));

    // fill dp table.
    for (int i = m-1; i > 0u; i--) {
        for (int j = n-1; j > 0u; j--) {
            if (strcmp(a[i], b[j]) == 0u)
                dp[i][j] = 1 + dp[i+1][j+1];
            else
                dp[i][j] = dp[i+1][j] > dp[i][j+1] ? dp[i+1][j] : dp[i][j+1];
        }
    }

    // backtrack to generate diff.
    int i = 0u, j = 0u;
    while (i < m && j < n) {
        if (strcmp(a[i], b[j]) == 0u) {
            diff_append(diff, " %s", a[i]);
            i++; j++;
        } else if (dp[i + 1u][j] >= dp[i][j + 1u]) {
            diff_append(diff, "- %s", a[i]);
            i++;
        } else {
            diff_append(diff, "+ %s", b[j]);
            j++;
        }
    }
    // remaining lines
    while (i < m) diff_append(diff, "- %s", a[i++]);
    while (j < n) diff_append(diff, "+ %s", b[j++]);

    // cleanup
    for (int k = 0; k <= m; k++)
        free(dp[k]);
    free(dp);
};

/**
 * @brief convert a diff type to a string representation.
 *
 * @param type the diff type to be converted.
 * @return a string representation of the diff type.
 */
char* diff_type_str(const e_diff_type_t type) {
    // switch case will suffice.
    switch (type) {
        /*! file diffs. */
        case E_DIFF_TYPE_NONE:
            return "none";
        case E_DIFF_NEW_FILE:
            return "nfile";
        case E_DIFF_FILE_DELETED:
            return "dfile";
        case E_DIFF_FILE_MODIFIED:
            return "mfile";

        /*! folder diffs. */
        case E_DIFF_NEW_FOLDER:
            return "ndir";
        case E_DIFF_FOLDER_DELETED:
            return "ddir";
        case E_DIFF_FOLDER_MODIFIED:
            return "mdir";

        /*! unknown diff type. */
        default:
            return "none";
    }
};

/**
 * @brief convert a diff type to a string representation.
 *
 * @param type the diff type to be converted.
 * @return a string representation of the diff type.
 */
e_diff_type_t diff_str_type(const char* string) {
    switch (string[0]) {
        // new something?
        case 'n': {
            if (string[1] == 'f') return E_DIFF_NEW_FILE;
            if (string[1] == 'd') return E_DIFF_NEW_FOLDER;
            return E_DIFF_TYPE_NONE;
        }

        // deleted something?
        case 'd': {
            if (string[1] == 'f') return E_DIFF_FILE_DELETED;
            if (string[1] == 'd') return E_DIFF_FOLDER_DELETED;
            return E_DIFF_TYPE_NONE;
        }

        // modified something?
        case 'm': {
            if (string[1] == 'f') return E_DIFF_FILE_MODIFIED;
            if (string[1] == 'd') return E_DIFF_FOLDER_MODIFIED;
            return E_DIFF_TYPE_NONE;
        }

        /*! default case, no diff type. */
        default: return E_DIFF_TYPE_NONE;
    }
};

/**
 * @brief create a file diff between two files (modified only + renaming).
 *
 * @param old filename of the old file (stored changes).
 * @param new filename of the new file we are comparing against.
 * @return a diff_t structure containing the new differences between the two files.
 */
diff_t diff_file_modified(const char* old, const char* new) {
    // create the two files in terms of data.
    FILE* f_old = fopen(old, "r");
    FILE* f_new = fopen(new, "r");
    if (!f_old || !f_new) {
        if (f_old) fclose(f_old);
        if (f_new) fclose(f_new);
        perror("fopen failed; could not open file(s) for reading.\n");
        return (diff_t) {
            .type = E_DIFF_TYPE_NONE,
            .s_name = 0x0,
            .n_name = 0x0,
            .count = 0x0,
            .capacity = 0x0,
            .lines = 0x0,
        };
    }

    // creating our diff., and then allocating the memory for the data.
    diff_t diff = {
        .type = E_DIFF_FILE_MODIFIED,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };
    diff.s_name = malloc(strlen(old) + 1u);
    strcpy(diff.s_name, old);
    diff.n_name = malloc(strlen(new) + 1u);
    strcpy(diff.n_name, new);

    // get the lines of the old file.
    int old_size = 0u;
    char** old_data = rindat(f_old, &old_size);
    if (!old_data) {
        fclose(f_old);
        fclose(f_new);
        perror("failed to read stored changes.\n");
        return diff;
    }
    int new_size = 0u;
    char** new_data = rindat(f_new, &new_size);
    if (!new_data) {
        for (int i = 0; i < old_size; i++)
            if (old_data[i]) free(old_data[i]);
        free(old_data);
        fclose(f_old);
        fclose(f_new);
        perror("failed to read new file.\n");
        return diff;
    }

    // use lcs to find the differences and store them in the diff structure.
    lcs(old_data, old_size, new_data, new_size, &diff);
    diff.hash = crc32((unsigned char*) new_data, new_size);

    // cleanup and return the diff.
    for (int i = 0; i < old_size; i++)
        if (old_data[i]) free(old_data[i]);
    free(old_data);
    fclose(f_old);
    for (int i = 0; i < new_size; i++)
        if (new_data[i]) free(new_data[i]);
    free(new_data);
    fclose(f_new);
    return diff;
};

/**
 * @brief create a file diff for a new file added.
 *
 * @param new filename of the new file we are adding
 * @return a diff_t structure containing the new differences made.
 */
diff_t diff_file_new(const char* new) {
    // open either the oldest, or the newest file for reading.
    FILE* f = fopen(new, "r");
    if (!f) {
        perror("fopen failed; could not open file for diff. reading.\n");
        return (diff_t) {
            .type = E_DIFF_TYPE_NONE,
            .s_name = 0x0,
            .n_name = 0x0,
            .count = 0x0,
            .capacity = 0x0,
            .lines = 0x0,
        };
    }

    // creating our diff., and then allocating the memory for the data.
    diff_t diff = {
        .type = E_DIFF_NEW_FILE,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };
    diff.n_name = malloc(strlen(new) + 1u);
    strcpy(diff.n_name, new);
    diff.s_name = malloc(2u);
    strcpy(diff.s_name, "0");

    // just a list of empty chars.
    char** empty;

    // capture the new data...
    int new_size = 0u;
    char** new_data = rindat(f, &new_size);
    if (!new_data) {
        fclose(f);
        perror("failed to read file changes.\n");
        return diff;
    }

    // run lcs and capture the lines.
    for (unsigned long i = 0u; i < new_size; i++) {
        diff_append(&diff, "+ %s", new_data[i]);
    }
    diff.hash = crc32((unsigned char*) new_data, new_size);
    fclose(f);
    return diff;
};

/**
 * @brief create a file diff for a recently deleted file.
 *
 * @param old_diff most recent commits diff on this file before it got 'deleted'.
 * @return a diff_t structure containing the deleted differences.
 */
diff_t diff_file_deleted(const diff_t* old_diff) {
    // creating our diff., and then allocating the memory for the data.
    diff_t diff = {
        .type = E_DIFF_FILE_DELETED,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };
    diff.s_name = strdup(old_diff->n_name);
    diff.n_name = malloc(2u);
    strcpy(diff.n_name, "0");

    // copy the old information from the most recent commit that deleted it.
    int old_size = 0u;
    char** old_lines = rollback_to_diff(old_diff, &old_size);
    // run lcs and capture the lines.
    for (unsigned long i = 0u; i < old_size; i++) {
        diff_append(&diff, "- %s", old_lines[i]);
    }
    diff.hash = crc32((unsigned char*) old_lines, old_size);
    return diff;
};

/**
 * @brief create a folder diff containing the type of diff.
 *
 * @param path the path to the folder.
 * @param type the diff type (modified = renamed).
 * @returns a diff_t structure containing the diff information.
 */
diff_t diff_folder(const char* path, const e_diff_type_t type) {
    // create a temporary diff_t structure.
    diff_t diff = {
        .type = type,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };

    // if it is a new folder, we simply write the new folder name and thats it.
    if (type == E_DIFF_NEW_FOLDER) {
        diff.n_name = malloc(strlen(path) + 1);
        strcpy(diff.n_name, path);
        diff.s_name = malloc(2u);
        strcpy(diff.s_name, "0");
        diff.hash = crc32((unsigned char*) &diff.n_name, strlen(path) + 1u);
        return diff;
    }
    // if it is a deleted folder, we simply write the old name and thats it.
    diff.s_name = malloc(strlen(path) + 1);
    strcpy(diff.s_name, path);
    diff.n_name = malloc(2u);
    strcpy(diff.n_name, "0");
    diff.hash = crc32((unsigned char*) &diff.s_name, strlen(path) + 1u);
    return diff;
};

/**
 * @brief create a folder diff containing the modified / renamed diff information.
 *
 * @param path the path to the folder.
 * @param new_path the new folder name.
 * @returns a diff_t structure containing the diff information.
 */
diff_t diff_folder_modified(const char* path, const char* new_path) {
    // create a temporary diff_t structure.
    diff_t diff = {
        .type = E_DIFF_FOLDER_MODIFIED,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };

    // capture the old and new path.
    diff.s_name = malloc(strlen(path) + 1);
    strcpy(diff.s_name, path);
    diff.n_name = malloc(strlen(new_path) + 1);
    strcpy(diff.n_name, new_path);
    diff.hash = crc32((unsigned char*) &diff.n_name, strlen(path) + 1u);
    return diff;
};

/**
 * @brief append a line to the diff structure.
 *
 * @param diff the diff_t structure to append to.
 * @param fmt the format string for the line.
 * @param ... the arguments for the format string.
 */
void diff_append(diff_t* diff, const char* fmt, ...) {
    if (diff->count >= diff->capacity) {
        diff->capacity = diff->capacity ? diff->capacity * 2u : 16u;
        diff->lines = realloc(diff->lines, sizeof(char*) * diff->capacity);
    }

    // create a buffer for the line.
    char buffer[MAX_LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // dup it over.
    diff->lines[diff->count++] = strdup(buffer);
};

/**
 * @brief write a .diff file to disk given the diff structure and path.
 *
 * @param diff the diff_t structure to write to disk.
 * @param path the path to write the diff file to.
 */
void diff_write(const diff_t* diff, const char* path) {
    // open the file for writing.
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("fopen failed; could not open file for writing.\n");
        exit(-1); // exit on failure.
    }

    // write the header, including the hash.
    fprintf(f, "type:%s\nstored:%s\nnew:%s\ncrc32:%u\n\n", diff_type_str(diff->type), \
        diff->s_name, diff->n_name, diff->hash);

    // if the type is none, or something to do with the folder,
    //  we haven't written anything and it can be ignored.
    if (diff->type == E_DIFF_TYPE_NONE || diff->type == E_DIFF_NEW_FOLDER || \
        diff->type == E_DIFF_FOLDER_MODIFIED || diff->type == E_DIFF_FOLDER_DELETED) {
        fclose(f);
        exit(-1); // exit on failure.
    }

    // then continuously write the lines.
    for (unsigned long i = 0u; i < diff->count; i++) {
        fprintf(f, "%s\n", diff->lines[i]);
    }
    fclose(f);
};

/**
 * @brief read a diff file from disk and return a diff structure.
 *
 * @param path the path to the diff file to read.
 * @return a diff structure containing the differences read from the diff file.
 */
diff_t diff_read(const char* path) {
    // create a temporary diff structure.
    diff_t diff = {
        .type = E_DIFF_TYPE_NONE,
        .s_name = 0x0,
        .n_name = 0x0,
        .count = 0x0,
        .capacity = 0x0,
        .lines = 0x0,
    };

    // open the file for reading.
    FILE* f = fopen(path, "r");
    if (!f) {
        perror("fopen failed; could not open file for reading.\n");
        exit(-1); // exit on failure.
    }

    // read the header first.
    char s_name[128u], n_name[128u], type[32u];
    ucrc32_t hash;
    int scanned = fscanf(f, "type:%31[^\n]\nstored:%127[^\n]\nnew:%127[^\n]\ncrc32:%u\n", \
        type, s_name, n_name, &hash);
    if (scanned != 4u) {
        perror("fscanf failed; could not read diff header.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }

    // allocate memory for the names.
    diff.s_name = malloc(strlen(s_name) + 1u);
    if (!diff.s_name) {
        perror("malloc failed; could not allocate memory for stored name.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }
    strcpy(diff.s_name, s_name);
    diff.n_name = malloc(strlen(n_name) + 1u);
    if (!diff.n_name) {
        perror("malloc failed; could not allocate memory for new name.\n");
        fclose(f);
        exit(-1); // exit on failure.
    }
    strcpy(diff.n_name, n_name);

    // copy over type and crc32 hash for new file.
    diff.type = diff_str_type(type);
    diff.hash = hash;

    // if the type is none, or something to do with the folder,
    //  we haven't written anything and it can be ignore.
    if (diff.type == E_DIFF_TYPE_NONE || diff.type == E_DIFF_NEW_FOLDER || \
        diff.type == E_DIFF_FOLDER_MODIFIED || diff.type == E_DIFF_FOLDER_DELETED) {
        fclose(f);
        exit(-1); // exit on failure.
    }

    // now read each of the lines in the diff file.
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), f)) {
        // strip newline.
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // append the line to the diff structure.
        diff_append(&diff, "%s", buffer);
    }

    // return the diff structure.
    fclose(f);
    return diff;
};
