#include "hashing.h"
#include "config.h"
#include "logging.h"
#include "parser.h"
#include "utils.h"
#include <dirent.h>
#include <fnmatch.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *exclude_patterns[] =
    {
        "/etc/mtab",
        "/etc/ld.so.cache",
        "/etc/machine-id",
        "/etc/NetworkManager/*",
        "/etc/ssl/*",
        "/etc/alternatives/*",
        "/etc/systemd/system/*.wants/*",
        "/etc/pki/tls/certs/*",
        "/etc/letsencrypt/*",
        "/etc/cups/*"};

static const size_t exclude_count = sizeof(exclude_patterns) / sizeof(exclude_patterns[0]);

EVP_MD_CTX *init_evp_context(const EVP_MD *type)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        log_message(LOG_ERR, "Failed to create EVP_MD_CTX of type: %s", type);

        return NULL;
    }

    if (EVP_DigestInit_ex(ctx, type, NULL) != 1)
    {
        log_message(LOG_ERR, "Failed to initialize EVP digest context");
        EVP_MD_CTX_free(ctx);

        return NULL;
    }

    return ctx;
}

bool is_excluded(const char *path, const char *patterns[], size_t pattern_count)
{
    for (size_t i = 0; i < pattern_count; i++)
    {
        if (fnmatch(patterns[i], path, 0) == 0)
            return true;
    }

    return false;
}

int binary_to_hex(const unsigned char *digest, unsigned int len, char out_hex[HASH_HEX_LEN + 1])
{
    static const char hex[] = "0123456789abcdef";
    for (unsigned int i = 0; i < len; i++)
    {
        out_hex[i * 2] = hex[(digest[i] >> 4) & 0xF];
        out_hex[i * 2 + 1] = hex[digest[i] & 0xF];
    }
    out_hex[len * 2] = '\0';

    return 0;
}

static void update_hash_with_metadata(EVP_MD_CTX *ctx, const char *path, const struct stat *st)
{
    EVP_DigestUpdate(ctx, path, strlen(path));
    EVP_DigestUpdate(ctx, &st->st_mode, sizeof(st->st_mode));
    EVP_DigestUpdate(ctx, &st->st_size, sizeof(st->st_size));
    EVP_DigestUpdate(ctx, &st->st_mtime, sizeof(st->st_mtime));
    EVP_DigestUpdate(ctx, &st->st_ctime, sizeof(st->st_ctime));
    EVP_DigestUpdate(ctx, &st->st_uid, sizeof(st->st_uid));
    EVP_DigestUpdate(ctx, &st->st_gid, sizeof(st->st_gid));
}

static int hash_file_sha256(const char *path, unsigned int *len, unsigned char digest[SHA256_DIGEST_LENGTH])
{
    FILE *fp = open_file(path);
    if (!fp)
        return -1;

    EVP_MD_CTX *ctx = init_evp_context(EVP_sha256());

    unsigned char buf[BUF_SIZE];
    size_t        n;
    struct stat   st;

    if (stat(path, &st) == -1)
    {
        perror("stat");
        return -1;
    }

    update_hash_with_metadata(ctx, path, &st);

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        EVP_DigestUpdate(ctx, buf, n);

    EVP_DigestFinal_ex(ctx, digest, len);
    EVP_MD_CTX_free(ctx);
    fclose(fp);

    return 0;
}

int sha256_file(const char *path, char out_hex[HASH_HEX_LEN + 1])
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  len;

    if (hash_file_sha256(path, &len, digest) != 0)
        return -1;

    binary_to_hex(digest, len, out_hex);

    return 0;
}

static int compare_names(const void *a, const void *b)
{
    const char *const *nameA = a;
    const char *const *nameB = b;

    return strcmp(*nameA, *nameB);
}

static int hash_directory_sha256(const char *dir_path, unsigned int *len, unsigned char digest[SHA256_DIGEST_LENGTH])
{
    EVP_MD_CTX *ctx;
    size_t      entry_count;
    char      **entries;

    ctx = init_evp_context(EVP_sha256());
    entry_count = 0;
    entries = get_all_entries(dir_path, &entry_count);
    if (!entries)
    {
        log_message(LOG_WARNING, "Skipping directory (unreadable): %s", dir_path);
        return -1;
    }

    qsort(entries, entry_count, sizeof(char *), compare_names);

    for (size_t i = 0; i < entry_count; i++)
    {
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entries[i]);

        if (is_excluded(fullpath, exclude_patterns, exclude_count))
        {
            // printf("Skipping excluded path: %s\n", fullpath);
            continue;
        }

        struct stat st;
        if (stat(fullpath, &st) == -1)
        {
            perror("stat");
            continue;
        }

        if (S_ISREG(st.st_mode))
        {
            unsigned int  file_len;
            unsigned char filehash[SHA256_DIGEST_LENGTH];

            hash_file_sha256(fullpath, &file_len, filehash);
            EVP_DigestUpdate(ctx, filehash, SHA256_DIGEST_LENGTH);
        }
        else if (S_ISDIR(st.st_mode))
        {
            unsigned int  dir_len;
            unsigned char dirhash[SHA256_DIGEST_LENGTH];

            hash_directory_sha256(fullpath, &dir_len, dirhash);
            EVP_DigestUpdate(ctx, dirhash, SHA256_DIGEST_LENGTH);
        }

        free(entries[i]);
    }
    free(entries);

    EVP_DigestFinal_ex(ctx, digest, len);
    EVP_MD_CTX_free(ctx);

    return 0;
}

int sha256_dir(const char *dir_path, char out_hex[HASH_HEX_LEN + 1])
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  len;

    if (hash_directory_sha256(dir_path, &len, digest) != 0)
        return -1;

    binary_to_hex(digest, len, out_hex);

    return 0;
}

void hash_file_lines(const char *path)
{
    FILE   *fp;
    char   *line;
    size_t  len;
    ssize_t read;
    int     line_no;

    fp = open_file(path);

    line = NULL;
    len = 0;
    line_no = 1;

    log_message(LOG_INFO, "Calculated hash per line for %s", path);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        unsigned char digest[SHA256_DIGEST_LENGTH];
        EVP_Digest(line, read, digest, NULL, EVP_sha256(), NULL);

        char out_hex[HASH_HEX_LEN + 1];
        binary_to_hex(digest, SHA256_DIGEST_LENGTH, out_hex);

        log_message(LOG_INFO, "Hash for line %d: %s\n", line_no, out_hex);
        line_no++;
    }
}

static void do_hash(config_entry_t entry)
{
    hash_level_t type = entry.hash_level;
    char         out_hex[HASH_HEX_LEN + 1];

    switch (type)
    {
        case HASH_DIR_LVL:
            if (sha256_dir(entry.path, out_hex) != 0)
                log_message(LOG_ERR, "Error hashing directory: %s", entry.path);
            else
                log_message(LOG_INFO, "Hash for %s: %s\n", entry.path, out_hex);
            break;
        case HASH_FILE_LVL:
            if (sha256_file(entry.path, out_hex) != 0)
                log_message(LOG_ERR, "Error hashing file: %s", entry.path);
            else
                log_message(LOG_INFO, "Hash for %s: %s\n", entry.path, out_hex);
            break;
        case HASH_LINE_LVL:
            hash_file_lines(entry.path);
            break;
        default:
            break;
    }
}

void integrity_check(void)
{
    size_t          capacity = 0;
    config_entry_t *entries;

    entries = parse_config(&capacity);

    for (size_t i = 0; i < capacity; i++)
    {
        do_hash(entries[i]);
    }

    free(entries);
}
