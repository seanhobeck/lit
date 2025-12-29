/**
 * @author Sean Hobeck
 * @date 2025-12-28
 */
#ifndef HASH_H
#define HASH_H

/* type definition for a sha1 hash, 20 bytes long. */
typedef unsigned char sha1_t[20ul];

/* type definition for a sha256 hash, 32 bytes long. */
typedef unsigned char sha256_t[32ul];

/* type definition for a crc32 hash, 10 bytes long. */
typedef unsigned int ucrc32_t;

/**
 * @brief generate a sha1 hash from the given data.
 *
 * @param data pointer to the data to hash.
 * @param size size of the data in bytes.
 * @param hash sha1_t structure to store the hash.
 */
void
sha1(const unsigned char* data, unsigned long size, sha1_t hash);

/**
 * @brief convert a sha1 hash to a string representation.
 *
 * @param hash the sha1 hash to be converted.
 * @return a string representation of the sha1 hash.
 */
char*
strsha1(const sha1_t hash);

/**
 * @brief generate a crc32 hash from the given data.
 *
 * @param data pointer to the data to hash.
 * @param size size of the data in bytes.
 * @return ucrc32_t structure to store the hash.
 */
ucrc32_t
crc32(const unsigned char* data, unsigned long size);

/**
 * @brief convert a crc32 hash to a string representation.
 *
 * @param hash the crc32 hash to be converted.
 * @return a string representation of the crc32 hash.
 */
char*
strcrc32(const ucrc32_t hash);
#endif /* HASH_H */
