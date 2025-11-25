#include "../include/utils.h"
#include "../include/logging.h"

DIR *open_directory(const char *directory_path)
{
    DIR *dir = opendir(directory_path);

    if (!dir)
    {
        log_message(LOG_ERR, "Error opening directory %s: %s", directory_path, strerror(errno));
        return NULL;
    }

    return dir;
}

FILE *open_file(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        log_message(LOG_ERR, "Error opening file %s: %s", filename, strerror(errno));
        return NULL;
    }

    return fp;
}

void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr && size != 0)
    {
        log_message(LOG_ERR, "Memory allocation failed (size: %zu bytes)", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *safe_realloc(void *ptr, size_t new_size)
{
    void *tmp = realloc(ptr, new_size);
    if (!tmp && new_size != 0)
    {
        log_message(LOG_ERR, "Memory reallocation failed (size: %zu bytes)", new_size);
        exit(EXIT_FAILURE);
    }
    return tmp;
}

char *safe_strdup(const char *s)
{
    if (!s)
    {
        log_message(LOG_ERR, "safe_strdup called with NULL pointer");
        exit(EXIT_FAILURE);
    }

    char *copy = strdup(s);
    if (!copy)
    {
        log_message(LOG_ERR, "String duplication failed for input: %s", s);
        exit(EXIT_FAILURE);
    }

    return copy;
}

char **get_all_entries(const char *dir_path, size_t *entry_count)
{
    DIR           *dir;
    struct dirent *entry;
    char         **entries;

    dir = open_directory(dir_path);
    if (!dir)
        return NULL;

    entries = NULL;
    *entry_count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        entries = safe_realloc(entries, (*entry_count + 1) * sizeof(char *));
        entries[*entry_count] = safe_strdup(entry->d_name);
        (*entry_count)++;
    }

    closedir(dir);
    return entries;
}
