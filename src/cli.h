/**
 * @author Sean Hobeck
 * @date 2025-08-12
 *
 * @file cli.h
 *    the cli module of lit, responsible for handling the arguments passed by the user and
 *    doing certain actions (CRUD, repository actions, etc).
 */
#ifndef CLI_H
#define CLI_H

/*! @uses arg_t */
#include "arg.h"

/**
 * @brief handle the arguments passed by the user as a cli (command-line interface) tool.
 *
 * @param args the arg_t structure created when parsing argc & argv.
 * @return the return code if needed.
 */
int
cli_handle(arg_t args);
#endif