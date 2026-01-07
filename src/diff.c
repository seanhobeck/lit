/**
 * @author Sean Hobeck
 * @date 2026-01-06
 */
#include "diff.h"

/*! @uses time, time_t. */
#include <time.h>

/*! @uses assert. */
#include <assert.h>

/*! @uses fopen, fprintf, FILE*, fseek, ftell, remove. */
#include <stdio.h>

/*! @uses calloc, free. */
#include <stdlib.h>

/*! @uses va_start, va_arg, va_list. */
#include <stdarg.h>

/*! @uses strcmp. */
#include <string.h>

/*! @uses internal. */
#include "utl.h"

/*! @uses llog, E_LOGGER_LEVEL_INFO. */
#include "log.h"

/*!~ @note this is a format for the main parts of data that are written at the start (header) of
 *  the file for a diff., stored within a commit, stored within a branch, within the repository. */
#define DIFF_HEADER_FORMAT "type:%d\nstored:%127[^\n]\nnew:%127[^\n]\ncrc32:%u\n"

/**
 * @brief calculate the crc32 hash of a file based on its contents.
 *
 * @param path the path to the file to be hashed.
 * @return a crc32 hash for the file.
 */
internal ucrc32_t
file_crc32(const char* path) {
    /* assert on the path. */
    assert(path != 0x0);

    /* read the file in. */
    FILE* file = fopen(path, "rb");
    if (!file) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open file for hashing");
        return 0;
    }

    /* get file size. */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size <= 0) {
        fclose(file);
        return 0;
    }

    /* allocate buffer for file contents. */
    unsigned char* buffer = calloc(1, file_size);
    if (!buffer) {
        llog(E_LOGGER_LEVEL_ERROR,"calloc failed; could not allocate buffer for file.");
        fclose(file);
        return 0;
    }

    /* read file contents. */
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    if (bytes_read != (size_t)file_size) {
        llog(E_LOGGER_LEVEL_ERROR, "fread; could not read entire file.\n");
        free(buffer);
        return 0;
    }

    /* calculate crc32. */
    ucrc32_t hash = crc32(buffer, file_size);
    free(buffer);
    return hash;
}

/**
 * @brief create the crc32 hash for a diff given its information
 *  (unique to the diff and not the file).
 *
 * @param diff the diff for the crc32 to be appended to.
 */
internal void
create_crc32(diff_t* diff) {
    /* assert on the diff ptr. */
    assert(diff != 0x0);

    /* allocate the file and open it. */
    char* tmp = calloc(1, 256);
    sprintf(tmp, "%lu.tmp", time(0x0));
    FILE* ftmp = fopen(tmp, "w");

    /* iterate through each line. */
    _foreach(diff->lines, char*, line)
        fprintf(ftmp, "%s\n", line);
    _endforeach;

    /* at the end add some of the diffs data for uniqueness. */
    fprintf(ftmp, "type:%d\nstored:%s\nnew:%s\nmtime:%lu\n", \
        diff->type, diff->stored_path, diff->new_path, time(0x0));
    fclose(ftmp);
    diff->crc = file_crc32(tmp);
    remove(tmp);
}

/**
 * @brief append a line to the diff structure.
 *
 * @param diff the diff_t structure to append to.
 * @param fmt the format string for the line.
 * @param ... the arguments for the format string.
 */
internal void
append_to_diff(diff_t* diff, const char* fmt, ...) {
    /* assert on the diff ptr, then the fmt. */
    assert(diff != 0x0);
    assert(fmt != 0x0);

    /* create a buffer for the line. */
    char buffer[MAX_LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);

    /* dup it over. */
    dyna_push(diff->lines, strdup(buffer));
}

/**
 * @brief calculate the longest common subsequence (lcs) between two strings.
 *
 * @param a the first string.
 * @param m the length of the first string.
 * @param b the second string.
 * @param n the length of the second string.
 * @param diff the diff_t structure to store the differences.
 */
internal void
lcs(char** a, const int m, char** b, const int n, diff_t* diff) {
    /* assert on all the ptrs. */
    assert(a != 0x0);
    assert(b != 0x0);
    assert(diff != 0x0);

    /* @ref[https://en.wikipedia.org/wiki/Longest_common_subsequence] */
    int **dp = malloc((m+1) * sizeof(int*));
    for (int i = 0; i <= m; i++)
        dp[i] = calloc(n+1, sizeof(int));

    /* fill dp table. */
    for (int i = m-1; i > 0; i--) {
        for (int j = n-1; j > 0; j--) {
            if (strcmp(a[i], b[j]) == 0)
                dp[i][j] = 1 + dp[i+1][j+1];
            else
                dp[i][j] = dp[i+1][j] > dp[i][j+1] ? dp[i+1][j] : dp[i][j+1];
        }
    }

    /* backtrack to generate diff. */
    int i = 0, j = 0;
    while (i < m && j < n) {
        if (strcmp(a[i], b[j]) == 0) {
            append_to_diff(diff, "%s", a[i]);
            i++; j++;
        } else if (dp[i + 1][j] >= dp[i][j + 1u]) {
            append_to_diff(diff, "- %s", a[i]);
            i++;
        } else {
            append_to_diff(diff, "+ %s", b[j]);
            j++;
        }
    }

    /* remaining lines. */
    while (i < m) append_to_diff(diff, "- %s", a[i++]);
    while (j < n) append_to_diff(diff, "+ %s", b[j++]);

    /* cleanup. */
    for (int k = 0; k <= m; k++)
        free(dp[k]);
    free(dp);
}


/**
 * @brief create a file diff between two files (modified only + renaming).
 *
 * @param old_path filename of the old file (stored changes).
 * @param new_path filename of the new file we are comparing against.
 * @return a diff_t structure containing the new differences between the two files.
 */
diff_t*
create_file_modified_diff(const char* old_path, const char* new_path) {
    /* assert on the paths. */
    assert(old_path != 0x0);
    assert(new_path != 0x0);

    /* create the two files in terms of data. */
    FILE* f_old = fopen(old_path, "r");
    FILE* f_new = fopen(new_path, "r");
    if (!f_old || !f_new) {
        /* close either file. */
        if (f_old) fclose(f_old);
        if (f_new) fclose(f_new);
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open file(s) for reading.\n");
        return 0x0;
    }

    /* creating our diff., and then allocating the memory for the data. */
    diff_t* diff = calloc(1, sizeof *diff);
    diff->type = E_DIFF_FILE_MODIFIED;

    /* creating the dynamic array. */
    diff->lines = dyna_create();

    /* copy over the names of the new and old path. */
    diff->stored_path = calloc(1, strlen(old_path) + 1);
    strcpy(diff->stored_path, old_path);
    diff->new_path = calloc(1, strlen(new_path) + 1);
    strcpy(diff->new_path, new_path);

    /* get the lines of the old file. */
    size_t old_size = 0;
    char** old_data = freadls(f_old, &old_size);
    if (!old_data) {
        fclose(f_old);
        fclose(f_new);
        llog(E_LOGGER_LEVEL_ERROR,"failed to read stored changes.\n");
        return diff;
    }
    size_t new_size = 0;
    char** new_data = freadls(f_new, &new_size);
    if (!new_data) {
        fclose(f_old);
        fclose(f_new);
        llog(E_LOGGER_LEVEL_ERROR,"failed to read new file.\n");
        return diff;
    }

    /* use lcs to find the differences and store them in the diff structure. */
    lcs(old_data, (int) old_size, new_data, (int) new_size, diff);

    /* write out to a temp file and read the hash, then close and remove it. */
    create_crc32(diff);

    /* cleanup and return the diff. */
    for (size_t i = 0; i < old_size; i++)
        if (old_data[i]) free(old_data[i]);
    free(old_data);
    fclose(f_old);
    for (size_t i = 0; i < new_size; i++)
        if (new_data[i]) free(new_data[i]);
    free(new_data);
    fclose(f_new);
    return diff;
}

/**
 * @brief create a file diff for a new/deleted file.
 *
 * @param path filepath of the file we are adding/deleting.
 * @param type enum diff type for either adding or deleting.
 * @return a diff_t structure containing the new differences made.
 */
diff_t*
create_file_diff(const char* path, e_diff_ty_t type) {
    /* assert on the path. */
    assert(path != 0x0);

    /* creating our diff., and then allocating the memory for the data. */
    diff_t* diff = calloc(1, sizeof *diff);
    diff->type = type;
    diff->stored_path = strdup(path);
    diff->new_path = strdup(path);

    /* creating the dynamic array. */
    diff->lines = dyna_create();

    /* copy the old information from the file before deleting it. */
    FILE* f = fopen(path, "r");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open file for diff. reading.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* run lcs and capture the lines. */
    size_t size = 0;
    char** lines = freadls(f, &size);
    for (size_t i = 0; i < size; i++) {
        if (diff->type == E_DIFF_FILE_DELETED) append_to_diff(diff, "- %s", lines[i]);
        else append_to_diff(diff, "+ %s", lines[i]);
    }

    /* write out to a temp file and read the hash, then close and remove it. */
    create_crc32(diff);
    return diff;
}

/**
 * @brief create a folder diff containing the type of diff.
 *
 * @param path the path to the folder.
 * @param type the diff type (modified = renamed).
 * @returns a diff_t structure containing the diff information.
 */
diff_t*
create_folder_diff(const char* path, const e_diff_ty_t type) {
    /* assert on the path. */
    assert(path != 0x0);

    /* create a temporary diff_t structure. */
    diff_t* diff = calloc(1, sizeof *diff);
    diff->type = type;

    /* creating the dynamic array. */
    diff->lines = dyna_create();

    /* write the new and stored path to be the same. */
    diff->new_path = calloc(1, strlen(path) + 1);
    strcpy(diff->new_path, path);
    diff->stored_path = calloc(1, strlen(path) + 1);
    strcpy(diff->stored_path, path);
    diff->crc = crc32((unsigned char*) diff->new_path, strlen(path) + 1);
    return diff;
}

/**
 * @brief write a .diff file to disk given the diff structure and path.
 *
 * @param diff the diff_t structure to write to disk.
 * @param path the path to write the diff file to.
 */
void
write_diff(const diff_t* diff, const char* path) {
    /* assert on the diff then the path. */
    assert(diff != 0x0);
    assert(path != 0x0);

    /* open the file for writing. */
    FILE* f = fopen(path, "w");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open file for writing.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* write the header, including the hash. */
    fprintf(f, "type:%d\nstored:%s\nnew:%s\ncrc32:%u\n\n", diff->type, \
        diff->stored_path, diff->new_path, diff->crc);

    /* if the type is none, or something to do with the folder,
       we haven't written anything, and it can be ignored. */
    if (diff->type == E_DIFF_TYPE_NONE || diff->type == E_DIFF_FOLDER_NEW || \
        diff->type == E_DIFF_FOLDER_MODIFIED || diff->type == E_DIFF_FOLDER_DELETED) {
        fclose(f);
        return;
    }

    /* then continuously write the lines. */
    _foreach(diff->lines, char*, line)
        fprintf(f, "%s\n", line);
    _endforeach;
    fclose(f);
}

/**
 * @brief read a diff file from disk and return a diff structure.
 *
 * @param path the path to the diff file to read.
 * @return a diff structure containing the differences read from the diff file.
 */
diff_t*
read_diff(const char* path) {
    /* assert on the path. */
    assert(path != 0x0);

    /* create a temporary diff structure. */
    diff_t* diff = calloc(1, sizeof *diff);

    /* open the file for reading. */
    FILE* f = fopen(path, "r");
    if (!f) {
        llog(E_LOGGER_LEVEL_ERROR,"fopen failed; could not open file for reading.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* read the header first. */
    diff->stored_path = calloc(1, 128);
    diff->new_path = calloc(1, 128);

    /* create the dynamically allocated list. */
    diff->lines = dyna_create();

    /* allocate memory for the names. */
    if (!diff->stored_path || !diff->new_path) {
        llog(E_LOGGER_LEVEL_ERROR,"calloc failed; could not allocate memory for diff header.\n");
        fclose(f);
        exit(EXIT_FAILURE); /* exit on failure. */
    }
    int scanned = fscanf(f, DIFF_HEADER_FORMAT, \
        &diff->type, diff->stored_path, diff->new_path, &diff->crc);
    if (scanned != 4) {
        llog(E_LOGGER_LEVEL_ERROR,"fscanf failed; could not read diff header.\n");
        exit(EXIT_FAILURE); /* exit on failure. */
    }

    /* if the type is none, or something to do with the folder,
       we haven't written anything, and it can be ignored. */
    if (diff->type == E_DIFF_TYPE_NONE || diff->type == E_DIFF_FOLDER_NEW || \
        diff->type == E_DIFF_FOLDER_MODIFIED || diff->type == E_DIFF_FOLDER_DELETED) {
        return diff;
    }

    /* we now read each of the lines in the diff file. */
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, 256, f)) {
        /* strip newline. */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        /* append the line to the diff structure. */
        append_to_diff(diff, "%s", buffer);
    }

    /* return the diff structure. */
    fclose(f);
    return diff;
}