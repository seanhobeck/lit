/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file diff.h
 *    the diff module, responsible for creating comparisons between files and folders,
 *    while also maintaining a history of changes in '.diff' files.
 */
#ifndef DIFF_H
#define DIFF_H

/*! @uses ucrc32_t, crc32. */
#include <hash.h>

// @note at the most 8192 lines of code can be stored in a file.
#define MAX_LINES 8192ul

// @note at the most 256 characters can be stored in a line.
#define MAX_LINE_LEN 256ul

/*!
 *  @details ^^^
 *
 *  these macros above specify that you cannot have a file that exceeds 8192 lines,
 *  with 256 characters per line. this means at the MOST you will be able to handle 2 MiB files.
 */

/// @note a enum to differentiate types of diffs.
typedef enum {
    E_DIFF_TYPE_NONE = 0,           // no differences made.

    /*! file diffs. */
    E_DIFF_NEW_FILE = 0x1,          // a new file was created.
    E_DIFF_FILE_DELETED = 0x2,      // a file was deleted.
    E_DIFF_FILE_MODIFIED = 0x4,     // a file was modified.

    /*! folder diffs. */
    E_DIFF_NEW_FOLDER = 0x10,       // a new folder was created.
    E_DIFF_FOLDER_DELETED = 0x20,   // a folder was deleted.
    E_DIFF_FOLDER_MODIFIED = 0x40,   // a folder was modified.
} e_diff_type_t;

/**
 * @brief duplicate a string.
 *
 * @param str the string to duplicate.
 * @return a pointer to the duplicated string, or 0x0 on failure.
 */
char* strdup(const char* str);

/**
 * @brief convert a diff type to a string representation.
 *
 * @param type the diff type to be converted.
 * @return a string representation of the diff type.
 */
char* diff_type_str(const e_diff_type_t type);

/**
 * @brief convert a diff type to a string representation.
 *
 * @param type the diff type to be converted.
 * @return a string representation of the diff type.
 */
e_diff_type_t diff_str_type(const char* string);

/// @note a data structure to hold difference (diff.) information.
typedef struct {
    e_diff_type_t type; // type of diff.
    char* s_name, *n_name; // stored filename, and new filename.
    long count, capacity; // size of the change in bytes.
    char** lines; // our change in lines.
    ucrc32_t hash; // hash of the diff.
} diff_t;

/**
 * @brief create a file diff between two files (modified only + renaming).
 *
 * @param old filename of the old file (stored changes).
 * @param new filename of the new file we are comparing against.
 * @return a diff_t structure containing the new differences between the two files.
 */
diff_t diff_file_modified(const char* old, const char* new);

/**
 * @brief create a file diff for a new file added.
 *
 * @param new filename of the new file we are adding
 * @return a diff_t structure containing the new differences made.
 */
diff_t diff_file_new(const char* new);

/**
 * @brief create a file diff for a recently deleted file.
 *
 * @param old_diff most recent commits diff on this file before it got 'deleted'.
 * @return a diff_t structure containing the deleted differences.
 */
diff_t diff_file_deleted(const diff_t* old_diff);

/**
 * @brief create a folder diff containing the type of diff.
 *
 * @param path the path to the folder.
 * @param type the diff type (modified should only be used in diff_folder_modified).
 * @returns a diff_t structure containing the diff information.
 */
diff_t diff_folder(const char* path, const e_diff_type_t type);

/**
 * @brief create a folder diff containing the modified / renamed diff information.
 *
 * @param path the path to the folder.
 * @param new_path the new folder name.
 * @returns a diff_t structure containing the diff information.
 */
diff_t diff_folder_modified(const char* path, const char* new_path);

/**
 * @brief create a folder diff between two filed (modified + remaining).

/**
 * @brief append a line to the diff structure.
 *
 * @param diff the diff_t structure to append to.
 * @param fmt the format string for the line.
 * @param ... the arguments for the format string.
 */
void diff_append(diff_t* diff, const char* fmt, ...);

/**
 * @brief write a .diff file to disk given the diff structure and path.
 *
 * @param diff the diff_t structure to write to disk.
 * @param path the path to write the diff file to.
 */
void diff_write(const diff_t* diff, const char* path);

/**
 * @brief read a diff file from disk and return a diff structure.
 *
 * @param path the path to the diff file to read.
 * @return a diff structure containing the differences read from the diff file.
 */
diff_t diff_read(const char* path);
#endif //DIFF_H
