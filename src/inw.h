/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#ifndef INW_H
#define INW_H

/*! @uses time_t, time. */
#include <time.h>

/*! @uses dyna_t, dyna_push, dyna_get. */
#include "dyna.h"

/**
 * enum for differentiating inode types, think the difference between a file and a folder.
 */
typedef enum {
    E_INODE_TYPE_FILE = 0x1, /* a file. */
    E_INODE_TYPE_FOLDER = 0x2, /* a folder. */
} e_inode_ty_t;

/**
 * a data structure representing a file or folder inode. this contains everything we need
 *  to know about the file, that is, its type, path, name, and modification time.
 */
typedef struct {
    e_inode_ty_t type; /* type of inode. */
    char* path, *name; /* path to the inode, and name of the inode. */
    time_t mtime; /* modification time. */
} inode_t;

/**
 * enum for differentiating inode walking types; the difference between recursing into
 *  subdirectories or not, todo; files & folders only within a directory.
 */
typedef enum {
    E_INW_TYPE_NO_RECURSE = 0x0,
    E_INW_TYPE_RECURSE = 0x1, /* recursively collect files in subdirectories. */
    E_INW_TYPE_FILES_ONLY = 0x2, /* collect only files within a directory. */
    E_INW_TYPE_FOLDERS_ONLY = 0x3, /* collect only folders within a directory. */
} e_inode_walk_ty_t;

/**
 * @brief walk through the directory for all inodes.
 *
 * @param path the path to the directory to walk within.
 * @param type the type of walking to be performed (no recurse or recurse).
 * @return a dynamic array for the inode data.
 */
dyna_t*
inw_walk(const char* path, e_inode_walk_ty_t type);
#endif /* INW_H */
