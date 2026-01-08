/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#ifndef ARG_H
#define ARG_H

/*! @uses dyna_t. */
#include "../core/dyna.h"

/**
 * enum for all proper argument types (commands), think 'lit commit' or 'lit init' or 'lit log', etc.
 *  these are all considered so-called 'proper' arguments, because they define the behavior of the
 *  program; also may have proceeding flag or parameter arguments to modify their behavior.
 */
typedef enum {
    E_PROPER_ARG_NONE = 0, /* no command type specified. */
    E_PROPER_ARG_INIT = 0x1, /* initialize a new repository. */
    E_PROPER_ARG_COMMIT = 0x2, /* commit changes to the repository. */
    E_PROPER_ARG_ROLLBACK = 0x3, /* rollback to a previous commit. */
    E_PROPER_ARG_CHECKOUT = 0x4, /* checkout a branch or commit. */
    E_PROPER_ARG_LOG = 0x5, /* show the log of the repository. */
    E_PROPER_ARG_CREATE_BRANCH = 0x6, /* create a branch. */
    E_PROPER_ARG_SWITCH_BRANCH = 0x7, /* switch to a branch. */
    E_PROPER_ARG_REBASE_BRANCH = 0x8, /* rebase a branch onto another. */
    E_PROPER_ARG_DELETE_BRANCH = 0x9, /* delete a branch. */
    E_PROPER_ARG_ADD_INODE = 0xa, /* add a file or folder to the branch. */
    E_PROPER_ARG_DELETE_INODE = 0xb, /* delete a file or folder from the branch. */
    E_PROPER_ARG_HELP = 0xc, /* show the help message. */
    E_PROPER_ARG_VERSION = 0xd, /* show the version of the program. */
    E_PROPER_ARG_CLEAR_CACHE = 0xe, /* clear the object cache. */
    E_PROPER_ARG_RESTORE = 0xf, /* restore the entire branch. */
    E_PROPER_ARG_ADD_TAG = 0x10, /* add a tag for a commit. */
    E_PROPER_ARG_DELETE_TAG = 0x11, /* delete a tag for a commit. */
} e_proper_arg_ty_t;

/**
 * enum for all flag argument types (modifiers), think '--all' or '--no-recurse' or '--soft', etc.
 *  these are all considered so-called 'flag' arguments, because they modify the behavior of
 *  a proper argument. they are responsible for providing additional context to the proper
 *  argument they are modifying, they are also very strictly enforced.
 */
typedef enum {
    E_FLAG_ARG_ALL = 0x0, /* --all with a proceeding subdirectory to be recursed over. */
    E_FLAG_ARG_NO_RECURSE = 0x1, /* --no-recurse with a proceeding subdirectory. */
    E_FLAG_ARG_HARD = 0x2, /* --hard flag for rollback/checkout. */
    E_FLAG_ARG_GRAPH = 0x3, /* --graph flag for log. */
    E_FLAG_ARG_FILTER = 0x4, /* --filter flag for log with a proceeding filter string. */
    E_FLAG_ARG_MAX_COUNT = 0x5, /* --max-count flag for a log with a proceeding integer. */
    E_FLAG_ARG_VERBOSE = 0x6, /* --verbose flag for log. */
    E_FLAG_ARG_QUIET = 0x7, /* --quiet flag for log. */
    E_FLAG_ARG_FROM = 0x8, /* --from flag for creation of a branch. */
    E_FLAG_ARG_MESSAGE = 0x9, /* --m | ch--message flag for creation of a commit. */
    E_FLAG_ARG_TAG = 0xa, /* --tag flag for rollback/checkout. */
} e_flag_arg_ty_t;

/**
 * enum for all parsed argument types (generic), think 'proper', 'flag', or 'parameter'. they
 *  can be one of three types:
 *      1. proper argument: a command that defines the behavior of the program.
 *      2. flag argument: a modifier that modifies the behavior of a proper argument.
 *      3. parameter argument: a value that is passed to a proper or flag argument
 */
typedef enum {
    E_PROPER_ARGUMENT = 0x0, /* -l/-log or -c/-commit */
    E_FLAG_TO_ARGUMENT = 0x1, /* --all . */
    E_PARAMETER_TO_ARGUMENT = 0x2, /* "commit message" or "file.txt" or "dev" */
} e_argument_ty_t;

/**
 * a data structure representing a parsed command line argument, which again, can be one of the three
 *  types, proper, flag, or parameter. this structure contains a 'details' field, which is a union of
 *  either a proper or flag argument type (for specification), and a 'value' field, which is a string
 *  representing the value of the argument (for parameters).
 */
typedef struct {
    e_argument_ty_t type; /* generic argument type. */
    union {
        e_proper_arg_ty_t proper;
        e_flag_arg_ty_t flag;
    } details; /* union of either proper or flag argument type (for specification). */
    char* value; /* the value of the argument (string). */
} argument_t;

/**
 * @brief parse command line arguments and return a dynamic array of all parsed arguments.
 *
 * @param argc the count of raw command line arguments.
 * @param argv the vector of raw command line arguments.
 * @return a dynamic array of all parsed arguments.
 */
dyna_t*
parse_arguments(int argc, char** argv);
#endif /* ARG_H */
