/**
 * @author Sean Hobeck
 * @date 2025-08-29
 *
 * @file conf.h
 *    the conf module of lit, responsible for handling configuration options.
 */
#ifndef CONF_H
#define CONF_H

/*! @uses bool, true, false */
#include <stdbool.h>

/// @note a data structure to hold configuration options read for lit.
typedef struct {
    bool debug;     /** whether to print debug output. */
} config_t;

/**
 * @brief read the configuration options from the .lit/config file.
 *
 * @return the config_t structure read from the file.
 */
config_t*
read_config();
#endif //CONF_H