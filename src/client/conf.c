/**
 * @author Sean Hobeck
 * @date 2026-01-06
 */
#include "../conf.h"

/*! @uses fprintf, fopen, fgets. */
#include <stdio.h>

/*! @uses strcmp. */
#include <string.h>

/*! @uses calloc */
#include <stdlib.h>

/**
 * @brief read the configuration options from the .lit/config file.
 *
 * @return the config_t structure read from the file.
 */
config_t*
read_config() {
    /* default configuration. */
    config_t* config = calloc(1, sizeof *config);
    *config = (config_t) {
        .debug = false
    };

    /* open the file for reading. */
    FILE* f = fopen(".lit/config", "r");
    if (!f) {
        /* could not open the file, return default config. */
        return config;
    }

    /* read the file line by line. */
    char line[256];
    while (fgets(line, 256, f)) {
        if (line[0] == '#' || line[0] == '\n') {
            /* skip comments and empty lines. */
            continue;
        }

        /* parse the line for key=value pairs. */
        char key[129], value[129];
        int scanned = sscanf(line, "%128[^=]=%128s", key, value);
        if (scanned == 2) {
            /* key-value pair options below... */

            /* debug option. */
            if (!strcmp(key, "debug"))
                config->debug = !strcmp(value, "true");
        }
    }

    /* return the configuration read. */
    return config;
}