/**
 * @author Sean Hobeck
 * @date 2025-09-01
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

/// @note a macro to define the version of the program.
#define VERSION "1.5.11"

/**
 * @brief print the help message for the command line arguments.
 */
void
help_args() {
    printf("usage: lit [-v | --version] [-h | --help] [-i | --init] [-c | --commit <msg>]\n"
           "\t[-r | --rollback <hash>] [-C | -checkout <hash>] [-s | --status] [-sB | --switch-branch <name>]\n"
           "\t[-dB | --delete-branch <name>] [-aB | --add-branch <name>] [-rB | --rebase-branch <src> <dest>]\n"
           "\t[-a | --add <path>] [-d <hash>| --delete <path>] [-m | --modified <path> :<new_path>] \n"
           "\t[-aT | --add-tag <hash> <name> ] [-dT | --delete-tag <name>] \n\n");

    // print out the options to the user. (disable warnings in ~/.lit/config with disable_warnings=1)
    printf("options:\n"
           "\t-v | --version\t\t\tprint the current version of the program.\n"
           "\t-h | --help\t\t\tprint this help message.\n"
           "\t-i | --i\t\t\tinitialize a new repository in the current working directory.\n"
           "\t-a | --add <path>\t\tadd a file or folder to the branch.\n"
           "\t-m | --modified <path> :(optional) <new_path>\tstate that a file has been changed.\n"
           "\t-d | --delete <path>\t\tdelete a file or folder from the branch.\n\n"
           "\t-r | --rollback <hash>\t\t*rollback to a previous commit.\n"
           "\t-C | --checkout <hash>\t\t*checkout a commit.\n"
           "\t-s | --status\t\t\tprint the status of the repository on the active branch.\n\n"
           "\t-aB | --add-branch <name>\t\tcreate a new branch to the repository.\n"
           "\t-sB | --switch-branch <name>\t\tswitch to a branch.\n"
           "\t-rB | --rebase-branch <src> <dest>\trebase a branch onto another.\n"
           "\t-dB | --delete-branch <name>\t\tdelete a branch from the repository.\n\n"
           "\t-aT | --add-tag <hash> <name>\t\tadd a tag to a commit on the active branch.\n"
           "\t-dT | --delete-tag <name>\t\tdelete a tag from the repository.\n\n\n"
           "any option with an asterisk (*) can produce a warning in stdout, to remove\n"
           " set disable_warnings=1 in configuration file at, \'~/.lit/config\'\n");
};

/**
 * @brief parse command line arguments and return a struct containing the parsed arguments.
 *
 * @param argc the argument count.
 * @param argv the argument vector.
 * @param cmd the command structure to be filled.
 */
void
parse_args(int argc, char** argv, arg_t* cmd) {
    // initialize our command structure to be empty.
    cmd->type = E_ARG_TYPE_NONE;
    cmd->argv = argv;

    // if there are no arguments, print the help message.
    if (argc < 2) {
        help_args();
        return;
    }

    // string compare the first argv and then the second if it is required.
    // --help or -h to print out the help message.
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        cmd->type = E_ARG_TYPE_HELP;
        help_args();
        return;
    }
    // --version or -v to print out the current version of the program.
    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        cmd->type = E_ARG_TYPE_VERSION;
        fprintf(stdout,"lit version %s\n", VERSION);
        return;
    }
    // --init or -i to initialize a new repository.
    if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--init")) {
        cmd->type = E_ARG_TYPE_INIT;
        return;
    }
    // --commit or -c to commit changes to the repository.
    if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--commit")) {
        cmd->type = E_ARG_TYPE_COMMIT;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing commit message.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // --rollback or -r to rollback to a previous commit.
    if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--rollback")) {
        cmd->type = E_ARG_TYPE_ROLLBACK;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing commit hash / name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // --checkout or -C to checkout a commit.
    if (!strcmp(argv[1], "-C") || !strcmp(argv[1], "--checkout")) {
        cmd->type = E_ARG_TYPE_CHECKOUT;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing commit hash / name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // --status or -s to show the status of the repository.
    if (!strcmp(argv[1], "-s") || !strcmp(argv[1], "--status")) {
        cmd->type = E_ARG_TYPE_STATUS;
        return;
    }
    // --add or -a to add a file or folder to the branch.
    if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--add")) {
        cmd->type = E_ARG_TYPE_ADD_INODE;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing file or folder path.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // --delete or -d to delete a file or folder from the branch.
    if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--delete")) {
        cmd->type = E_ARG_TYPE_DELETE_INODE;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing file or folder path.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // --modified or -m to state that a file has been changed.
    if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--modified")) {
        cmd->type = E_ARG_TYPE_MODIFIED_INODE;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr,"argc < 3; missing file or folder path.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -aB or --add-branch
    if (!strcmp(argv[1], "-aB") || !strcmp(argv[1], "--add-branch")) {
        cmd->type = E_ARG_TYPE_CREATE_BRANCH;

        // if there is no argv[2], print the help message.
        if (argc < 3) {
            fprintf(stderr, "argc < 3; missing branch name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -dB or --delete-branch
    if (!strcmp(argv[1], "-dB") || !strcmp(argv[1], "--delete-branch")) {
        cmd->type = E_ARG_TYPE_DELETE_BRANCH;
        // if there is no extra_cmd argument, print the help message.
        if (argc < 3) {
            fprintf(stderr, "argc < 3; missing branch name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -sB or --switch-branch
    if (!strcmp(argv[1], "-sB") || !strcmp(argv[1], "--switch-branch")) {
        cmd->type = E_ARG_TYPE_SWITCH_BRANCH;

        // if there is no argv[2], print the help message.
        if (argc < 3) {
            fprintf(stderr, "argc < 3; missing branch name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -rB or --rebase-branch
    if (!strcmp(argv[1], "-rB") || !strcmp(argv[1], "--rebase-branch")) {
        cmd->type = E_ARG_TYPE_REBASE_BRANCH;

        // if there is no argv[2], print the help message.
        if (argc < 4) {
            fprintf(stderr, "argc < 4; missing branch names.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -cc or --clear-cache
    if (!strcmp(argv[1], "-cc") || !strcmp(argv[1], "--clear-cache")) {
        cmd->type = E_ARG_TYPE_CLEAR_CACHE;
        return;
    }
    // -aT or --add-tag
    if (!strcmp(argv[1], "-aT") || !strcmp(argv[1], "--add-tag")) {
        cmd->type = E_ARG_TYPE_ADD_TAG;

        // if the argument count does not match, fail.
        if (argc < 4) {
            fprintf(stderr, "argc < 4; missing tag name, and hash. \n");
            exit(EXIT_FAILURE);
        }
        return;
    }
    // -dT or --delete-tag
    if (!strcmp(argv[1], "-dT") || !strcmp(argv[1], "--delete-tag")) {
        cmd->type = E_ARG_TYPE_DELETE_TAG;

        // if the argument count does not match, fail.
        if (argc < 3) {
            fprintf(stderr, "argc < 3; missing tag name.\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    // unrecognized argument/command, exit(EXIT_FAILURE).
    printf("unrecognized argument: %s\n", argv[1]);
    printf("use -h or --help to see the list of available commands.\n");
    exit(EXIT_FAILURE);
};