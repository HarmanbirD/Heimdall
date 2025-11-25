#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DIR   *open_directory(const char *directory_path);
FILE  *open_file(const char *filename);
void  *safe_malloc(size_t size);
void  *safe_realloc(void *ptr, size_t new_size);
char  *safe_strdup(const char *s);
char **get_all_entries(const char *dir_path, size_t *entry_count);

#endif // FILE_UTILS_H
