/**
 * @author Sean Hobeck
 * @date 2026-01-08
 */
#ifndef NET_H
#define NET_H

#include <stddef.h>

/* packet magic to ensure that it has transferred accurately. */
#define PACKET_MAGIC 65552832123298150ul

/**
 * a data structure for a generic packet within transfer of information.
 */
typedef struct {
    size_t len, magic; /*  */
    unsigned char* buffer; /*  */
} packet_t;

#endif /* NET_H */