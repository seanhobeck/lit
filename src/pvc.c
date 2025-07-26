/**
 * @author Sean Hobeck
 * @date 2025-07-23
 *
 * @file pvc.c
 *    the path vector module, responsible for snapshotting
 *    the current state of the working directory into a vector.
 */
#include <pvc.h>

/*! @uses mkdir, getcwd. */
#include <sys/types.h>
#include <sys/stat.h>

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*! @uses strdup. */
#include <diff.h>

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
void push(pvc_t* vector, const pvc_inode_t inode) {
    if (vector->count == vector->cap) {
        vector->cap = vector->cap ? vector->cap * 2u : 16u;
        vector->nodes = realloc(vector->nodes, sizeof(pvc_inode_t) * vector->cap);
    }
    vector->nodes[vector->count++] = inode;
};

/**
 * @brief collect files in the current working directory and its subdirectories
 *
 * @param path the path to the directory to collect files from.
 * @param type the type of collection to perform (no recurse or recurse).
 * @return a pvc_t structure containing the collected files.
 */
pvc_t pvc_collect(const char* path, const e_pvc_type_t type) {
    /// create a new pvc_t structure to hold the files.
    pvc_t pvc = {
        .nodes= 0x0,
        .count = 0u,
        .cap = 0u,
    };

    // using POSIX compliance we can open the directory and read its contents.
    pdir_t d = opendir(path);
    if (!d) return pvc;

    // iterate through the directory entries.
    pdir_ent_t ent;
    while ((ent = readdir(d))) {
        // skip the current and parent directory entries.
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        // get the full path of the entry.
        char npath[512u];
        snprintf(npath, sizeof(npath), "%s/%s", path, ent->d_name);

        // check the entry type, file vs folder.
        struct stat st;
        if (stat(npath, &st) == -1) continue;

        // create a new pvc_inode_t structure for the entry.
        pvc_inode_t inode = {
            .path = strdup(npath), // duplicate the path.
            .name = strdup(ent->d_name), // duplicate the name.
            .type = (S_ISDIR(st.st_mode)) ? E_PVC_INODE_TYPE_FOLDER : E_PVC_INODE_TYPE_FILE, // set the type.
        };

        // push it to our vector.
        push(&pvc, inode);

        // if we are recursing.
        if (type == E_PVC_TYPE_RECURSE) {
            if (S_ISDIR(st.st_mode)) {
                // for every file in this new vector, we want to
                // copy it over to this pvc and then free it.
                pvc_t recursed = pvc_collect(npath, type);
                for (unsigned long i = 0u; i < recursed.count; i++) {
                    push(&pvc, recursed.nodes[i]);
                }
                // free the recursed memory.
                free(recursed.nodes);
            }
        }
    }

    // cleanup
    closedir(d);
    return pvc;
};