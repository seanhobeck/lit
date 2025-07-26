/**
 * @author Sean Hobeck
 * @date 2025-07-22
 *
 * @file pvc.h
 *    the path vector module, responsible for snapshotting
 *    the current state of the working directory into a vector.
 */
#ifndef PVC_H
#define PVC_H

/// @note a enum to differentiate types of files and folders.
typedef enum {
    E_PVC_INODE_TYPE_FILE = 0x1, // a file.
    E_PVC_INODE_TYPE_FOLDER = 0x2, // a folder.
} e_pvc_inode_type_t;

/// @note a data structure to hold a file / folder temporarily.
typedef struct {
    char* path, *name; // path to the inode, and name of the inode.
    e_pvc_inode_type_t type; // type of inode.
} pvc_inode_t;

/// @note a data structure to hold a path vector, this will contain
///     inodes within a directory, as well as a count and capacity for the inode list.
typedef struct {
    pvc_inode_t* nodes; // array of inodes in the directory.
    unsigned long count, cap; // number of files in the directory.
} pvc_t;

/// @note a enum to differentiate types of path vector collection.
typedef enum {
    E_PVC_TYPE_NO_RECURSE = 0x0,
    E_PVC_TYPE_RECURSE = 0x1, // recursively collect files in subdirectories.
} e_pvc_type_t;

/**
 * @brief collect files in the current working directory and its subdirectories
 *
 * @param path the path to the directory to collect files from.
 * @param type the type of collection to perform (no recurse or recurse).
 * @return a pvc_t structure containing the collected files.
 */
pvc_t pvc_collect(const char* path, const e_pvc_type_t type);
#endif //PVC_H
