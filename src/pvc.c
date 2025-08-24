/**
 * @author Sean Hobeck
 * @date 2025-08-23
 *
 * @file pvc.c
 *    the path vector module, responsible for snapshotting
 *    the current state of the working directory into a vector.
 */
#include "pvc.h"

/*! @uses mkdir, getcwd. */
#include <sys/types.h>
#include <sys/stat.h>

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*! @uses strdup. */
#include "diff.h"

/*!@ uses malloc, free. */
#include <stdlib.h>

/*! @uses strcpy. */
#include <string.h>

/*! @uses dirent, opendir. */
#include <dirent.h>

/// @note a typedefinition for a directory
typedef DIR* pdir_t;

/// @note a typedefinition for a directory entry
typedef struct dirent* pdir_ent_t;

/**
 * @brief push a inode into the pvc_t structure.
 *
 * @param vector the pvc_t structure to which the inode will be pushed.
 * @param inode the pvc_inode_t structure to be pushed.
 */
void
vector_push(vector_t* vector, const vinode_t* inode) {
    // assert if the vector is null, not the nodes.
    assert(vector != 0x0);
    if (vector->count == vector->cap) {
        vector->cap = vector->cap ? vector->cap * 2u : 16u;
        vinode_t** nodes = realloc(vector->nodes, sizeof(vinode_t*) * vector->cap);
        if (!nodes) {
            fprintf(stderr, "realloc failed; could not reallocate memory for nodes.\n");
            exit(EXIT_FAILURE);
        }
        vector->nodes = nodes;
    }
    vector->nodes[vector->count++] = (vinode_t*) inode;
};

/**
 * @brief collect files in the current working directory and its subdirectories
 *
 * @param path the path to the directory to collect files from.
 * @param type the type of collection to perform (no recurse or recurse).
 * @return a pvc_t structure containing the collected files.
 */
vector_t*
vector_collect(const char* path, const e_vector_collect_result_t type) {
    // create a new pvc_t structure to hold the files.
    vector_t* pvc = calloc(1, sizeof *pvc);

    // using POSIX compliance we can open the directory and read its contents.
    pdir_t d = opendir(path);
    if (!d) return pvc;

    // iterate through the directory entries.
    pdir_ent_t ent;
    while ((ent = readdir(d))) {
        // skip the current and parent directory entries.
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        // get the full path of the entry.
        char* filepath = calloc(1, 512);
        snprintf(filepath, 512, "%s/%s", path, ent->d_name);

        // check the entry type, file vs folder.
        struct stat st;
        if (stat(filepath, &st) == -1) continue;

        // create a new pvc_inode_t structure for the entry.
        vinode_t* inode = calloc(1, sizeof *inode);
        *inode = (vinode_t) {
            .path = strdup(filepath), // duplicate the path.
            .name = strdup(ent->d_name), // duplicate the name.
            .type = (S_ISDIR(st.st_mode)) ? E_PVC_INODE_TYPE_FOLDER : E_PVC_INODE_TYPE_FILE, // set the type.
            .mtime = st.st_mtime, // set the modification time.
        };

        // push it to our vector.
        vector_push(pvc, inode);

        // if we are recursing.
        if (type == E_PVC_TYPE_RECURSE) {
            if (S_ISDIR(st.st_mode)) {
                // for every file in this new vector, we want to
                // copy it over to this pvc and then free it.
                vector_t* recursed = vector_collect(filepath, type);
                for (unsigned long i = 0u; i < recursed->count; i++) {
                    vector_push(pvc, recursed->nodes[i]);
                }
                // free the recursed memory.
                free(recursed);
            }
        }
    }

    // cleanup
    closedir(d);
    return pvc;
};

/**
 * @brief frees the entire allocated vector provided.
 *
 * @param vector the vector to be freed.
 */
void
vector_free(vector_t* vector) {
    // assert if the vector is none.
    assert(vector != 0x0);
    for (size_t i = 0; i < vector->count; i++) {
        free(vector->nodes[i]->path);
        free(vector->nodes[i]->name);
        free(vector->nodes[i]);
    }
    free(vector);
};
