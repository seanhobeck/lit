/**
 * @author Sean Hobeck
 * @date 2025-11-12
 *
 * @file inw.c
 *    the inode walking module, responsible for collecting files and folders
 *    within a given directory into a dynamic array.
 */
#include "inw.h"

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

/// @note a type definition for a directory
typedef DIR* pdir_t;

/// @note a type definition for a directory entry
typedef struct dirent* pdir_ent_t;

/**
 * @brief walk through the directory for all inodes.
 *
 * @param path the path to the directory to walk within.
 * @param type the type of walking to be performed (no recurse or recurse).
 * @return a dynamic array for the inode data.
 */
dyna_t*
inw_walk(const char* path, const e_inode_walk_ty_t type) {
    // create a new pvc_t structure to hold the files.
    dyna_t* array = dyna_create(sizeof(inode_t*));

    // we can open the directory and read its contents.
    pdir_t d = opendir(path);
    if (!d) return array;

    // iterate through the directory entries.
    pdir_ent_t ent;
    while ((ent = readdir(d))) {
        // skip the current and parent directory entries.
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        // get the full path of the entry.
        char* filepath = calloc(1, 512);
        snprintf(filepath, 512, "%s/%s", path, ent->d_name);

        // check the entry type, file or folder.
        struct stat st;
        if (stat(filepath, &st) == -1) continue;

        // create a new pvc_inode_t structure for the entry.
        inode_t* inode = calloc(1, sizeof *inode);
        *inode = (inode_t) {
            .path = strdup(filepath), // duplicate the path.
            .name = strdup(ent->d_name), // duplicate the name.
            .type = (S_ISDIR(st.st_mode)) ? E_INODE_TYPE_FOLDER : E_INODE_TYPE_FILE, // set the type.
            .mtime = st.st_mtime, // set the modification time.
        };

        // push it to our vector.
        dyna_push(array, inode);

        // if we are recursing.
        if (type == E_INW_TYPE_RECURSE) {
            if (S_ISDIR(st.st_mode)) {
                // for every file in this new vector, we want to
                // copy it over to this pvc and then free it.
                dyna_t* recursed = inw_walk(filepath, type);
                for (unsigned long i = 0u; i < recursed->length; i++)
                    dyna_push(array, dyna_get(recursed, i));

                // free the recursed memory.
                free(recursed);
            }
        }
    }

    // cleanup
    closedir(d);
    return array;
};