/**
 * @author Sean Hobeck
 * @date 2026-01-09
 */
#ifndef NET_H
#define NET_H

/*! @uses size_t. */
#include <stddef.h>

/* packet magic to ensure that it has transferred accurately. */
#define PACKET_MAGIC 65552832123298150ul

/** enum for differentiation of different packet types, ie. handshakes, data, responses, etc... */
typedef enum {
    E_PACKET_HANDSHAKE_C2S = 0x1, /* client-to-server handshake. */
} e_packet_ty_t;

/**
 * a data structure for a generic packet used within transfer of information. this structure can
 *  be used for simplification of data transfer, we simply read the packet and then open a
 *  memory stream with a file descriptor on that buffer and handle information that way instead
 *  of directly on the sockets file descriptor.
 */
typedef struct {
    size_t len, magic; /* length of the packet, as well as magic. */
    unsigned char* buffer; /* buffer of .length bytes. */
    e_packet_ty_t type; /* type of packet. */
} packet_t;

#endif /* NET_H */