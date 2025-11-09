#ifndef HASHING_H
#define HASHING_H

#include "config.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>

#define BUF_SIZE 65536

typedef enum
{
    HASH_DIR_LVL,  // Hash recursively
    HASH_FILE_LVL, // Single file
    HASH_LINE_LVL  // Per-line hashing
} hash_level_t;

typedef struct
{
    char  *path;
    off_t  size;
    time_t mtime;
    char   sha256[65];
} entry_t;

void        integrity_check(void);
int         sha256_file(const char *path, char out_hex[HASH_HEX_LEN + 1]);
int         sha256_dir(const char *dir_path, char out_hex[HASH_HEX_LEN + 1]);
void        hash_file_lines(const char *path);
int         binary_to_hex(const unsigned char *digest, unsigned int len, char out_hex[HASH_HEX_LEN + 1]);
bool        is_excluded(const char *path, const char *patterns[], size_t pattern_count);
EVP_MD_CTX *init_evp_context(const EVP_MD *type);

#endif // HASHING_H
