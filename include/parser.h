#ifndef PARSER_H
#define PARSER_H

#include "alert.h"
#include "hashing.h"
#include <limits.h>
#include <stddef.h>

typedef struct
{
    char          path[PATH_MAX];
    hash_level_t  hash_level;
    alert_level_t alert_level;
} config_entry_t;

config_entry_t *parse_config(size_t *count_out);

#endif // PARSER_H
