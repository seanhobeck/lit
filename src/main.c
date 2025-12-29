/**
 * @author Sean Hobeck
 * @date 2025-12-15
 */
#include "cli.h"

/** @brief entry point libc. */
int main(int argc, char **argv) {
    /* parse our commandline arguments. */
    dyna_t* argument_array = parse_arguments(argc, argv);

    /* we let the command-line interface handle everything. */
    return cli_handle(argument_array);
};