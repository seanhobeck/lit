/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#ifndef CONF_H
#define CONF_H

/*! @uses bool, true, false. */
#include <stdbool.h>

/**
 * a data structure for the configuration file that is loaded for the version control system.
 *  this contains all the information that the user would specify, note most of these commands
 *  can override any command specified in the cli (command-line-interface).
 */
typedef struct {
    bool debug; /* whether to print debug output. */
} config_t;

/**
 * @brief read the configuration options from the .lit/config file.
 *
 * @return the config_t structure read from the file.
 */
config_t*
read_config();
#endif /* CONF_H */