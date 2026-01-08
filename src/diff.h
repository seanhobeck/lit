/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef DIFF_H
#define DIFF_H

/*! @uses ucrc32_t, crc32. */
#include "hash.h"

/*! @uses dyna_t, dyna_push, etc... */
#include "dyna.h"

/**
 * enum to differentiate types of diffs/ changes; the difference between deleting,
 *  creating, and modifying both files and folders. each holds different applications for when
 *  operations are applied to commits containing diffs/ changes with these respective types.
 */
typedef enum {
    E_DIFF_TYPE_NONE = 0, /* no differences made. */
    E_DIFF_FILE_NEW = 0x1, /* a new file was created. */
    E_DIFF_FILE_DELETED = 0x2, /* a file was deleted. */
    E_DIFF_FILE_MODIFIED = 0x3, /* a file was modified. */
    E_DIFF_FOLDER_NEW = 0x4, /* a new folder was created. */
    E_DIFF_FOLDER_DELETED = 0x5, /* a folder was deleted. */
    E_DIFF_FOLDER_MODIFIED = 0x6, /* a folder was modified. */
} e_diff_ty_t;

/**
 * a data structure to hold diff. (file difference) information. the information stored often has
 *  to do with the changing of a crc32 hash as a unique identifier, paths, lines, as well as a
 *  dynamic array for the changes made to the file specified.
 */
typedef struct {
    e_diff_ty_t type; /* type of diff. */
    char* stored_path, *new_path; /* stored and new filepath. */
    dyna_t* lines; /* change in lines. */
    ucrc32_t crc; /* hash of the diff. */
} diff_t;

/**
 * @brief create a file diff between two files (modified only + renaming).
 *
 * @param old_path filename of the old file (stored changes).
 * @param new_path filename of the new file we are comparing against.
 * @return a diff_t structure containing the new differences between the two files.
 */
diff_t*
create_file_modified_diff(const char* old_path, const char* new_path);

/**
 * @brief create a file diff for a new/deleted file.
 *
 * @param path filepath of the file we are adding/deleting.
 * @param type enum diff type for either adding or deleting.
 * @return a diff_t structure containing the new differences made.
 */
diff_t*
create_file_diff(const char* path, e_diff_ty_t type);

/**
 * @brief create a folder diff containing the type of diff.
 *
 * @param path the path to the folder.
 * @param type the diff type (modified should only be used in diff_folder_modified).
 * @returns a diff_t structure containing the diff information.
 */
diff_t*
create_folder_diff(const char* path, e_diff_ty_t type);

/**
 * @brief write a .diff file to disk given the diff structure and path.
 *
 * @param diff the diff_t structure to write to disk.
 * @param path the path to write the diff file to.
 */
void
write_diff(const diff_t* diff, const char* path);

/**
 * @brief read a diff file from disk and return a diff structure.
 *
 * @param path the path to the diff file to read.
 * @return a diff structure containing the differences read from the diff file.
 */
diff_t*
read_diff(const char* path);
#endif /* DIFF_H */
