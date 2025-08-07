/**
 * @author Sean Hobeck
 * @date 2025-07-30
 *
 * @file arg.h
 *      the argument module, responsible for parsing command line arguments
 *      from <argc> and <argv> and returning a struct containing the parsed arguments.
 */
#include "arg.h"

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*! @uses strcmp, strlen, strncpy */
#include <string.h>

/*! @uses exit, malloc */
#include <stdlib.h>

/*! @uses strdup */
#include "diff.h"

/// @note a macro to define the version of the program.
#define VERSION "1.3.9"

/**
 * @brief print the help message for the command line arguments.
 */
void help_args() {
    printf("usage: lit [-v | --version] [-h | --help] [-i | --init] [-c | --commit <message>]\n"
           "\t\t[-r | --rollback] [-C | -checkout] [-s | --status] [-b | --branch <name>]\n"
           "\t\t[-dB | --delete-branch <name>] [-cB | --create-branch] [-a | --add <path>] \n"
           "\t\t[-d | --delete <path>] | [-m | --modified <path>, opt: <new_path>] \n");
}

/**
 * @brief parse command line arguments and return a struct containing the parsed arguments.
 *
 * @param argc the argument count.
 * @param argv the argument vector.
 * @param cmd the command structure to be filled.
 */
void parse_args(int argc, char** argv, cmd_t* cmd) {
    // initialize our command structure to be empty.
    cmd->type = E_CMD_TYPE_NONE;
    cmd->is_tui = false;
    cmd->extra_cmd = 0x0;
    cmd->extra_data = 0x0;

    // if there are no arguments, print the help message.
    if (argc < 2) {
        help_args();
        return;
    }

    // string compare the first argv and then the second if it is required.
    // --help or -h to print out the help message.
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        cmd->type = E_CMD_TYPE_HELP;
        help_args();
        return;
    }
    // --version or -v to print out the current version of the program.
    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        cmd->type = E_CMD_TYPE_VERSION;
        printf("lit version %s\n", VERSION);
        return;
    }
    // --init or -i to initialize a new repository.
    if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--init")) {
        cmd->type = E_CMD_TYPE_INIT;
        return;
    }
    // --commit or -c to commit changes to the repository.
    if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--commit")) {
        cmd->type = E_CMD_TYPE_COMMIT;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            printf("argc < 3; missing commit message.\n");
            exit(-1);
        }
        // copy the commit message into the command structure.
        cmd->extra_cmd = strdup(argv[2]);
        return;
    }
    // --rollback or -r to rollback to a previous commit.
    if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--rollback")) {
        cmd->type = E_CMD_TYPE_ROLLBACK;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            printf("argc < 3; missing commit hash / name.\n");
            exit(-1);
        }
        // copy the commit message into the command structure.
        cmd->extra_cmd = strdup(argv[2]);
        return;
    }
    // --checkout or -C to checkout a commit.
    if (!strcmp(argv[1], "-C") || !strcmp(argv[1], "--checkout")) {
        cmd->type = E_CMD_TYPE_CHECKOUT;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            printf("argc < 3; missing commit hash / name.\n");
            exit(-1);
        }
        // copy the commit hash or name into
        cmd->extra_cmd = strdup(argv[2]);
        return;
    }
    // --status or -s to show the status of the repository.
    if (!strcmp(argv[1], "-s") || !strcmp(argv[1], "--status")) {
        cmd->type = E_CMD_TYPE_STATUS;
        return;
    }
    // --add or -a to add a file or folder to the branch.
    if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--add")) {
        cmd->type = E_CMD_TYPE_ADD_INODE;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            printf("argc < 3; missing file or folder path.\n");
            exit(-1);
        }
        // copy the file or folder path into the command structure.
        cmd->extra_cmd = strdup(argv[2]);
        return;
    }
    // --delete or -d to delete a file or folder from the branch.
    if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--delete")) {
        cmd->type = E_CMD_TYPE_DELETE_INODE;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            printf("argc < 3; missing file or folder path.\n");
            exit(-1);
        }
        // copy the file or folder path into the command structure.
        cmd->extra_cmd = strdup(argv[2]);
        return;
    }
    // --modified or -m to state that a file has been changed.
    if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--modified")) {
        cmd->type = E_CMD_TYPE_MODIFIED_INODE;
        // if there is no extra_cmd argument, pritn the help message.
        if (argc < 3) {
            printf("argc < 3; missing file or folder path.\n");
            exit(-1);
        }
        // copy over the modified file or folder path into the command structure.
        cmd->extra_cmd = strdup(argv[2]);
        if (argc >= 4) {
            // perhaps copy over a new filename.
            cmd->extra_data = strdup(argv[3]);
        }
        return;
    }

    /// TODO: branch merging, branch creation and deletion.
    ///
    /// the idea is that we will have a staging/ folder in .lit/ that will hold
    ///     the information about unstaged diffs / changes, and then for switching
    ///     we will have to create a field for the current checked out branch in
    ///     '.lit/repository' and then for merges we will have to slap all of the
    ///     commits from one branch ontop of the other, change the timestamps on
    ///     them to be the timestamp of the merge command, and then write.
    ///
    /// ...

    // unrecognized argument/command, exit(-1).
    printf("unrecognized argument: %s\n", argv[1]);
    printf("use -h or --help to see the list of available commands.\n");
    exit(-1);
};