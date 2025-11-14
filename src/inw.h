/**
 * @author Sean Hobeck
 * @date 2025-11-13
 *
 * @file inw.h
 *    the inode walking module, responsible for collecting files and folders
 *    within a given directory into a dynamic array.
 */
#ifndef INW_H
#define INW_H

/*! @uses time_t, time. */
#include <time.h>

/*! @uses dyna_t, dyna_push, dyna_get. */
#include "dyna.h"

/// @note an enum for differentiating inode types.
typedef enum {
    E_INODE_TYPE_FILE = 0x1, // a file.
    E_INODE_TYPE_FOLDER = 0x2, // a folder.
} e_inode_ty_t;

/// @note a data structure to hold inode data temporarily.
typedef struct {
    e_inode_ty_t type; // type of inode.
    char* path, *name; // path to the inode, and name of the inode.
    time_t mtime; // modification time.
} inode_t;

/// @note an enum to differentiate types of inode walking.
typedef enum {
    E_INW_TYPE_NO_RECURSE = 0x0,
    E_INW_TYPE_RECURSE = 0x1, // recursively collect files in subdirectories.
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
#endif //INW_H
