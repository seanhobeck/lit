/**
 * @author Sean Hobeck
 * @date 2025-08-15
 *
 * @file arg.h
 *      the argument module, responsible for parsing command line arguments
 *      from <argc> and <argv> and returning a struct containing the parsed arguments.
 */
#ifndef ARG_H
#define ARG_H

/// @note a enum to differentiate command types.
typedef enum {
    E_ARG_TYPE_NONE = 0,                // no command type specified.
    E_ARG_TYPE_INIT = 0x1,              // initialize a new repository.
    E_ARG_TYPE_COMMIT = 0x2,            // commit changes to the repository.
    E_ARG_TYPE_ROLLBACK = 0x3,          // rollback to a previous commit.
    E_ARG_TYPE_CHECKOUT = 0x4,          // checkout a branch or commit.
    E_ARG_TYPE_STATUS = 0x5,            // show the status of the repository.
    E_ARG_TYPE_CREATE_BRANCH = 0x6,     // create a branch.
    E_ARG_TYPE_SWITCH_BRANCH = 0x7,     // switch to a branch.
    E_ARG_TYPE_REBASE_BRANCH = 0x8,     // rebase a branch onto another..
    E_ARG_TYPE_DELETE_BRANCH = 0x9,     // delete a branch.
    E_ARG_TYPE_ADD_INODE = 0xa,         // add a file or folder to the branch.
    E_ARG_TYPE_MODIFIED_INODE = 0xb,    // mark a file or folder as modified.
    E_ARG_TYPE_DELETE_INODE = 0xc,      // delete a file or folder from the branch.
    E_ARG_TYPE_HELP = 0xd,              // show help message.
    E_ARG_TYPE_VERSION = 0xe,           // show the version of the program.
    E_ARG_TYPE_CLEAR_CACHE = 0xf,       // clear the object cache.
    E_ARG_TYPE_ADD_TAG = 0x10,          // add a tag for a commit.
    E_ARG_TYPE_DELETE_TAG = 0x11,       // delete a tag for a commit.
} e_arg_type_t;

/// @note a data structure to hold a single command.
typedef struct {
    e_arg_type_t type;  // type of command.
    char **argv;
    int argc;
    // extra data for the command, such as a branch name or commit message.
} arg_t;

/**
 * @brief parse command line arguments and return a struct containing the parsed arguments.
 *
 * @param argc the argument count.
 * @param argv the argument vector.
 * @param cmd the command structure to be filled.
 */
void
parse_args(int argc, char** argv, arg_t* cmd);
#endif //ARG_H
