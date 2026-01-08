/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#include "arg.h"

/*! @uses printf, perror, fopen, fclose, fscanf, sprintf. */
#include <stdio.h>

/*! @uses strcmp, strlen, strncpy */
#include <string.h>

/*! @uses exit, malloc */
#include <stdlib.h>

/*! @uses bool, true, false */
#include <stdbool.h>

/*! @uses internal. */
#include "core/utl.h"

/*! @uses llog, E_LOGGER_LEVEL_ERROR. */
#include "core/log.h"

/* program version macro. */
#define VERSION "1.16.07"

/* if we are expecting a parameter_argument. */
#define expected_parameter_argument(number_of_expected_args) \
    if (i + number_of_expected_args >= (size_t)argc) { \
        llog(E_LOGGER_LEVEL_ERROR, "expected parameter argument(s) after '%s'\n", cli_arg); \
        exit(EXIT_FAILURE); \
    }

/* check if a proper argument has already been captured in the dynamic array (cannot have more than one). */
#define check_if_proper_already_captured() \
    if (captured_proper) { \
        llog(E_LOGGER_LEVEL_ERROR, "only one proper argument can be specified per command line invocation.\n"); \
        exit(EXIT_FAILURE); \
    } else { \
        captured_proper = true; \
    }

/* add the value to the parsed argument. */
#define add_value_to_parsed_argument() \
    parsed_arg->value = strdup(cli_arg);

/**
 * @brief print the help message for the command line arguments.
 */
internal
void help_args() {
    /* print out the usage.*/
    llog(E_LOGGER_LEVEL_INFO,
           "usage: lit [-v | version] [-h | help] [-i | init] [-c | commit]\n"
           "\t[-r | rollback <hash>] [-C | -checkout <hash>] [-l | log] [-sB | switch-branch <name>]\n"
           "\t[-dB | delete-branch <name>] [-aB | add-branch <name>] [-rB | rebase-branch <src> <dest>]\n"
           "\t[-a | add <path>] [-d <hash>| delete <path>] \n"
           "\t[-aT | add-tag <hash> <name> ] [-dT | delete-tag <name>] [-cc | clear-cache]\n\n");

    /* print out the options to the user. (disable warnings in ~/.lit/config with disable_warnings=1) */
    llog(E_LOGGER_LEVEL_INFO,
           "\t-v | version\t\t\tprint the version of the program.\n"
           "\t-h | help\t\t\tprint this help message.\n"
           "\t-i | init\t\t\tinitialize a new repository.\n"
           "\t-a | add <path>\t\t\tadd a file or folder.\n"
           "\t-d | delete <path>\t\tdelete a file or folder.\n"
           "\t-c | commit\t\t\tcommit changes to the repository.\n\n"
           "\t-r | rollback <hash>\t\t*rollback to a previous commit.\n"
           "\t-C | checkout <hash>\t\t*checkout a newer commit.\n"
           "\t-l | log\t\t\tlog data from the repository.\n\n"
           "\t-aB | add-branch <name>\t\tcreate a new branch.\n"
           "\t-sB | switch-branch <name>\tswitch to a branch.\n"
           "\t-rB | rebase-branch <src> <dst> rebase a branch onto another.\n"
           "\t-dB | delete-branch <name>\tdelete a branch.\n\n"
           "\t-aT | add-tag <hash> <name>\tadd a tag to a commit.\n"
           "\t-dT | delete-tag <name>\t\tdelete a tag.\n\n"
           "\t-cc | clear-cache\tclear any cache leftover from previous operations.\n\n"
           "any option with an asterisk (*) can produce a warning in stdout, to remove\n"
           " set disable_warnings=1 in configuration file at, \'~/.lit/config\'\n"
           " note that all flag arguments (-verbose, -quiet, etc.) override your  config.\n");
}

/**
 * @brief parse command line arguments and return a dynamic array of all parsed arguments.
 *
 * @param argc the count of raw command line arguments.
 * @param argv the vector of raw command line arguments.
 * @return a dynamic array of all parsed arguments.
 */
dyna_t*
parse_arguments(int argc, char** argv) {
    /* create a dynamic array, then traverse the vector of cli args. */
    dyna_t* array = dyna_create();
    bool captured_proper = false;
    for (size_t i = 1; i < (size_t)argc; i++) {
        char* cli_arg = argv[i];

        /* create a parsed argument and parse. */
        argument_t* parsed_arg = calloc(1, sizeof(argument_t));

        /* proper arguments. */
        if (!strcmp(cli_arg, "-v") || !strcmp(cli_arg, "version")) {
            check_if_proper_already_captured();
            printf("lit version: %s\n", VERSION);
            exit(EXIT_SUCCESS);
        }
        if (!strcmp(cli_arg, "-h") || !strcmp(cli_arg, "help")) {
            check_if_proper_already_captured();
            help_args();
            exit(EXIT_SUCCESS);
        }
        if (!strcmp(cli_arg, "-i") || !strcmp(cli_arg, "init")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_INIT;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "-c") || !strcmp(cli_arg, "commit")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_COMMIT;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "-r") || !strcmp(cli_arg, "rollback")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_ROLLBACK;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-C") || !strcmp(cli_arg, "checkout")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_CHECKOUT;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-l") || !strcmp(cli_arg, "log")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_LOG;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "-a") || !strcmp(cli_arg, "add")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_ADD_INODE;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-d") || !strcmp(cli_arg, "delete")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_DELETE_INODE;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-aB") || !strcmp(cli_arg, "add-branch")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_CREATE_BRANCH;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-sB") || !strcmp(cli_arg, "switch-branch")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_SWITCH_BRANCH;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-rB") || !strcmp(cli_arg, "rebase-branch")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_REBASE_BRANCH;
            add_value_to_parsed_argument();
            expected_parameter_argument(2);
            goto _push;
        }
        if (!strcmp(cli_arg, "-dB") || !strcmp(cli_arg, "delete-branch")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_DELETE_BRANCH;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "-cc") || !strcmp(cli_arg, "clear-cache")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_CLEAR_CACHE;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "-rs") || !strcmp(cli_arg, "restore")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_RESTORE;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "-aT") || !strcmp(cli_arg, "add-tag")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_ADD_TAG;
            add_value_to_parsed_argument();
            expected_parameter_argument(2);
            goto _push;
        }
        if (!strcmp(cli_arg, "-dT") || !strcmp(cli_arg, "delete-tag")) {
            check_if_proper_already_captured();
            parsed_arg->type = E_PROPER_ARGUMENT;
            parsed_arg->details.proper = E_PROPER_ARG_DELETE_TAG;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }

        /* flag arguments. */
        if (!captured_proper) {
            llog(E_LOGGER_LEVEL_ERROR, "a flag argument cannot be specified before a proper argument.\n");
            exit(EXIT_FAILURE);
        }
        if (!strcmp(cli_arg, "--all")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_ALL;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "--no-recurse")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_NO_RECURSE;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "--hard")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_HARD;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "--graph")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_GRAPH;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "--filter")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_FILTER;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "--max-count")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_MAX_COUNT;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "--verbose")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_VERBOSE;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "--quiet")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_QUIET;
            add_value_to_parsed_argument();
            goto _push;
        }
        if (!strcmp(cli_arg, "--from")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_FROM;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "--message") || !strcmp(cli_arg, "--m")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_MESSAGE;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }
        if (!strcmp(cli_arg, "--tag")) {
            parsed_arg->type = E_FLAG_TO_ARGUMENT;
            parsed_arg->details.flag = E_FLAG_ARG_TAG;
            add_value_to_parsed_argument();
            expected_parameter_argument(1);
            goto _push;
        }

        /* otherwise we assume it is a parameter argument. */
        parsed_arg->type = E_PARAMETER_TO_ARGUMENT;
        add_value_to_parsed_argument();

        /* push the parsed argument to the array. */
        _push:
        dyna_push(array, parsed_arg);
    }
    return array;
}