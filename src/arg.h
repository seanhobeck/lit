/**
 * @author Sean Hobeck
 * @date 2025-07-30
 *
 * @file arg.h
 *      the argument module, responsible for parsing command line arguments
 *      from <argc> and <argv> and returning a struct containing the parsed arguments.
 */
#ifndef ARG_H
#define ARG_H

/*! @uses bool, true, false. */
#include <stdbool.h>

/// @note a enum to differentiate command types.
typedef enum {
    E_CMD_TYPE_NONE = 0,                // no command type specified.
    E_CMD_TYPE_INIT = 0x1,              // initialize a new repository.
    E_CMD_TYPE_COMMIT = 0x2,            // commit changes to the repository.
    E_CMD_TYPE_ROLLBACK = 0x3,          // rollback to a previous commit.
    E_CMD_TYPE_CHECKOUT = 0x4,          // checkout a branch or commit.
    E_CMD_TYPE_STATUS = 0x5,            // show the status of the repository.
    E_CMD_TYPE_CREATE_BRANCH = 0x6,     // create a branch.
    E_CMD_TYPE_SWITCH_BRANCH = 0x7,     // switch to a branch.
    E_CMD_TYPE_MERGE_BRANCH = 0x8,      // merge two branches.
    E_CMD_TYPE_DELETE_BRANCH = 0x9,     // delete a branch.
    E_CMD_TYPE_ADD_INODE = 0xa,         // add a file or folder to the branch.
    E_CMD_TYPE_MODIFIED_INODE = 0xb,    // mark a file or folder as modified.
    E_CMD_TYPE_DELETE_INODE = 0xc,      // delete a file or folder from the branch.
    E_CMD_TYPE_HELP = 0xd,              // show help message.
    E_CMD_TYPE_VERSION = 0xe,           // show the version of the program.
} e_cmd_type_t;

/// @note a data structure to hold a single command.
typedef struct {
    e_cmd_type_t type;  // type of command.
    bool is_tui;        // true if the command is being run in tui(text ui) mode, false otherwise.
    char* extra_cmd, *extra_data;        // extra data for the command, such as a branch name or commit message.
} cmd_t;

/**
 * @brief parse command line arguments and return a struct containing the parsed arguments.
 *
 * @param argc the argument count.
 * @param argv the argument vector.
 * @param cmd the command structure to be filled.
 */
void parse_args(int argc, char** argv, cmd_t* cmd);
#endif //ARG_H
