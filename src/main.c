/**
 * @author Sean Hobeck
 * @date 2025-08-13
 *
 * @file main.c
 *    entry point for the program.
 */
#include "cli.h"

/** @brief entry point libc. */
int main(int argc, char **argv) {
    // parse our commandline arguments.
    arg_t args;
    parse_args(argc, argv, &args);

    // we let the command-line interface handle this.
    return cli_handle(args);
};