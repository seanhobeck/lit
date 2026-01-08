/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#ifndef CLI_H
#define CLI_H

/*! @uses arg_t */
#include "arg.h"

/**
 * @brief handle the arguments passed by the user as a cli (command-line interface) tool.
 *
 * @param argument_array the dynamic array of arguments passed by the user.
 * @return the return code if needed (0 if not required).
 */
int
cli_handle(dyna_t* argument_array);
#endif /* CLI_H */