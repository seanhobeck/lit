/**
 * @author Sean Hobeck
 * @date 2025-08-22
 *
 * @file pvc.h
 *    the path vector module, responsible for snapshotting
 *    the current state of the working directory into a vector.
 */
#ifndef PVC_H
#define PVC_H

/*! @uses time_t, time */
#include <time.h>

/// @note a enum to differentiate types of files and folders.
typedef enum {
    E_PVC_INODE_TYPE_FILE = 0x1, // a file.
    E_PVC_INODE_TYPE_FOLDER = 0x2, // a folder.
} e_vinode_ty_t;

/// @note a data structure to hold a file / folder temporarily.
typedef struct {
    char* path, *name; // path to the inode, and name of the inode.
    time_t mtime; // modification time.
    e_vinode_ty_t type; // type of inode.
} vinode_t;

/// @note a data structure to hold a path vector, this will contain
///     inodes within a directory, as well as a count and capacity for the inode list.
typedef struct {
    vinode_t** nodes; // array of inodes in the directory.
    size_t count, cap; // number of files in the directory.
} vector_t;

/// @note a enum to differentiate types of path vector collection.
typedef enum {
    E_PVC_TYPE_NO_RECURSE = 0x0,
    E_PVC_TYPE_RECURSE = 0x1, // recursively collect files in subdirectories.
} e_vector_collect_result_t;

/**
 * @brief push a inode into the pvc_t structure.
 *
 * @param vector the pvc_t structure to which the inode will be pushed.
 * @param inode the pvc_inode_t structure to be pushed.
 */
void
vector_push(vector_t* vector, const vinode_t* inode);

/**
 * @brief frees the entire allocated vector provided.
 *
 * @param vector the vector to be free'd.
 */
void
vector_free(vector_t* vector);

/**
 * @brief collect files in the current working directory and its subdirectories
 *
 * @param path the path to the directory to collect files from.
 * @param type the type of collection to perform (no recurse or recurse).
 * @return a pvc_t structure containing the collected files.
 */
vector_t*
vector_collect(const char* path, const e_vector_collect_result_t type);
#endif //PVC_H
