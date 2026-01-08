/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#include "hash.h"

/*! @uses calloc, free, itoa. */
#include <stdlib.h>

/*! @uses memcpy. */
#include <string.h>

/*! @uses sprintf. */
#include <stdio.h>

/*! @uses assert */
#include <assert.h>

/* macro for rotating bits to the left in a 32-bit integer. */
#define rotl32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/* macro for rotating bits to the right in a 32-bit integer. */
#define rotr32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

/* macro for shifting bits to the right in a 32-bit integer. */
#define shr32(x, n) ((x) >> (n))

/* macro for logical functions from fips-180-2 standard. */
#define ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/*!
 * @brief logical functions from fips-180-4 standard.
 *
 * @ref[https://csrc.nist.gov/files/pubs/fips/180-2/final/docs/fips180-2.pdf]
 */
#define sigma0(x) (rotr32((x), 2) ^ rotr32((x), 13) ^ shr32((x), 22))
#define sigma1(x) (rotr32((x), 6) ^ rotr32((x), 11) ^ shr32((x), 25))
#define theta0(x) (rotr32((x), 7) ^ rotr32((x), 18) ^ shr32((x), 3))
#define theta1(x) (rotr32((x), 17) ^ rotr32((x), 19) ^ shr32((x), 10))

/*! @brief first 32 bits of the cube roots of the primes 2..64. */
static const unsigned int k256[64u] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/**
 * @brief generate a sha1 hash from the given data.
 *
 * @param data pointer to the data to hash.
 * @param size size of the data in bytes.
 * @param hash sha1_t structure to store the hash.
 */
void
sha1(const unsigned char* data, unsigned long size, sha1_t hash) {
    /* assert on the parameters. */
    assert(data != 0x0);
    assert(size > 0);

    /* initial hash values (sha1 standard). */
    unsigned int h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
        h3 = 0x10325476, h4 = 0xC3D2E1F0;

    /* compute padded msg len (mod 64). */
    unsigned long padded_len = (size + 9u + 63) / 64 * 64;
    /* allocate memory for the buffer, and copy the data into it (append '1' bit). */
    unsigned char* msg = calloc(padded_len, 1);
    memcpy(msg, data, size);
    msg[size] = 0x80;

    /* append original message length in bits as big endian. */
    unsigned long bit_len = size * 8;
    for (unsigned long i = 0; i < 8; i++)
        msg[padded_len - 1 - i] = (unsigned char) (bit_len >> (i * 8));

    /* process the message in 512-bit chunks (64 bytes). */
    for (unsigned long offset = 0; offset < padded_len; offset += 64) {
        unsigned words[80];

        /* 16 big-endian 32-bit words. */
        for (unsigned long i = 0; i < 16; i++)
            words[i] = (msg[offset + 4 * i] << 24) | (msg[offset + 4 * i + 1] << 16) | \
                (msg[offset + 4 * i + 2] << 8) | (msg[offset + 4 * i + 3]);

        /* extend to our 80 words. */
        for (unsigned long i = 16; i < 80; i++)
            words[i] = rotl32(words[i - 3] ^ words[i - 8] ^ \
                words[i - 14] ^ words[i - 16], 1);

        /* main compression loop. */
        unsigned int a = h0, b = h1, c = h2, d = h3, e = h4;
        for (unsigned long i = 0; i < 80; i++) {
            unsigned int f, k;
            if (i < 20u) {
                f = (b & c) | ((~b) & d);
                k = 0x5a827999;
            } else if (i < 40u) {
                f = b ^ c ^ d;
                k = 0x6ed9eba1;
            } else if (i < 60u) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8f1bbcdc;
            } else {
                f = b ^ c ^ d;
                k = 0xca62c1d6;
            }
            unsigned int j = rotl32(a, 5) + f + e + k + words[i];
            e = d; d = c; c = rotl32(b, 30); b = a; a = j;
        }

        /* add compressed chunk to our current hash values. */
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }
    free(msg);

    /* produce the final hash but now in big-endian. */
    const unsigned int arr[] = { h0, h1, h2, h3, h4 };
    for (unsigned long i = 0; i < 5; i++)
        for (unsigned long j = 0; j < 4; j++)
            hash[i * 4 + j] = (unsigned char) (arr[i] >> (24 - j * 8));
}

/**
 * @brief convert a sha1 hash to a string representation.
 *
 * @param hash the sha1 hash to be converted.
 * @return a string representation of the sha1 hash.
 */
char*
strsha1(const sha1_t hash) {
    /* assert on the hash. */
    assert(hash != 0x0);

    /* convert the sha1 hash to a string representation. */
    char* str = malloc(41); /* 40 hex chars + null terminator. */
    for (unsigned long i = 0u; i < 20; i++)
        sprintf(str + i * 2, "%02x", hash[i]);
    str[40] = '\0'; /* null-terminate the string. */
    return str;
}

/**
 * @brief generate a crc32 hash from the given data.
 *
 * @param data pointer to the data to hash.
 * @param size size of the data in bytes.
 * @return ucrc32_t structure to store the hash.
 */
ucrc32_t
crc32(const unsigned char* data, unsigned long size) {
    /* assert on the data. */
    assert(data != 0x0);
    assert(size > 0);

    /* according to ieee 802.3, @link[https://docs.amd.com/v/u/en-US/xapp209]. */
    ucrc32_t crc = ~0u;
    for (unsigned long i = 0; i < size; i++) {
        crc ^= data[i];
        for (unsigned long j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
        }
    }
    return ~crc;
}

/**
 * @brief convert a crc32 hash to a string representation.
 *
 * @param hash the crc32 hash to be converted.
 * @return a string representation of the crc32 hash.
 */
char*
strcrc32(const ucrc32_t hash) {
    /* assert on the hash. */
    assert(hash != 0x0);
    char* string = calloc(1, 11); /* 10 hex chars + null terminator. */
    sprintf(string, "%d", hash);
    return string;
}