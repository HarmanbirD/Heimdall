#include "parser.h"
#include "config.h"
#include "hashing.h"
#include "logging.h"
#include "utils.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void add_default_entries(config_entry_t **entries, size_t *count, size_t *capacity)
{
    const struct
    {
        const char   *path;
        hash_level_t  level;
        alert_level_t alert;
    } defaults[] = {
        {CONFIG_PATH, HASH_FILE_LVL, ALERT_RED},
        {"/etc/shadow", HASH_FILE_LVL, ALERT_RED},
        {"/etc/shadow", HASH_LINE_LVL, ALERT_RED},
        {"/etc/passwd", HASH_FILE_LVL, ALERT_YELLOW},
        {"/etc/group", HASH_FILE_LVL, ALERT_YELLOW},
        {"/etc/gshadow", HASH_FILE_LVL, ALERT_RED},
        {"/etc/sudoers", HASH_FILE_LVL, ALERT_RED},
        {"/etc/sudoers.d/", HASH_DIR_LVL, ALERT_RED},
        {"/etc/ssh/sshd_config", HASH_FILE_LVL, ALERT_RED},
        {"/etc/ssh/ssh_config", HASH_FILE_LVL, ALERT_YELLOW},
        {"/etc/systemd/system/", HASH_DIR_LVL, ALERT_RED},
        {"/etc/sysctl.conf", HASH_FILE_LVL, ALERT_YELLOW},
        {"/etc/hostname", HASH_FILE_LVL, ALERT_GREEN},
        {"/etc/hosts", HASH_FILE_LVL, ALERT_GREEN},
        {"/etc/resolv.conf", HASH_FILE_LVL, ALERT_GREEN},
        {"/etc/crontab", HASH_FILE_LVL, ALERT_RED},
        {"/etc/ssl/certs/", HASH_DIR_LVL, ALERT_YELLOW},
        {"/etc/pki/", HASH_DIR_LVL, ALERT_YELLOW},
        {"/etc/pam.d/", HASH_DIR_LVL, ALERT_RED},
    };

    const size_t default_count = sizeof(defaults) / sizeof(defaults[0]);

    for (size_t i = 0; i < default_count; i++)
    {
        if (*count >= *capacity)
        {
            *capacity *= 2;
            *entries = safe_realloc(*entries, *capacity * sizeof(config_entry_t));
        }

        strcpy((*entries)[*count].path, defaults[i].path);
        (*entries)[*count].hash_level = defaults[i].level;
        (*entries)[*count].alert_level = defaults[i].alert;
        (*count)++;
    }
}

static void load_user_config(config_entry_t **entries, size_t *count, size_t *capacity)
{
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (!fp)
    {
        log_message(LOG_INFO, "No user config file found at %s. Defaults only.", CONFIG_PATH);
        return;
    }

    char line[PATH_MAX + 16];
    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#' || strlen(line) < 3)
            continue;

        line[strcspn(line, "\n")] = '\0';

        char path[PATH_MAX];
        char level_char, alert_char;

        if (sscanf(line, " %[^,], %c, %c", path, &level_char, &alert_char) != 3)
        {
            log_message(LOG_ERR, "Malformed config line: %s", line);
            continue;
        }

        if (*count >= *capacity)
        {
            *capacity *= 2;
            *entries = realloc(*entries, *capacity * sizeof(config_entry_t));
            if (!*entries)
            {
                log_message(LOG_ERR, "Memory allocation failed in load_user_config()");
                fclose(fp);
                exit(EXIT_FAILURE);
            }
        }

        strncpy((*entries)[*count].path, path, sizeof((*entries)[*count].path) - 1);
        (*entries)[*count].path[sizeof((*entries)[*count].path) - 1] = '\0';

        switch (level_char)
        {
            case 'd': (*entries)[*count].hash_level = HASH_DIR_LVL; break;
            case 'f': (*entries)[*count].hash_level = HASH_FILE_LVL; break;
            case 'l': (*entries)[*count].hash_level = HASH_LINE_LVL; break;
            default:
                log_message(LOG_WARNING, "Unknown hash level '%c' in %s", level_char, path);
                continue;
        }

        switch (alert_char)
        {
            case 'r': (*entries)[*count].alert_level = ALERT_RED; break;
            case 'y': (*entries)[*count].alert_level = ALERT_YELLOW; break;
            case 'g': (*entries)[*count].alert_level = ALERT_GREEN; break;
            default:
                log_message(LOG_WARNING, "Unknown alert level '%c' in %s", alert_char, path);
                continue;
        }

        (*count)++;
    }

    fclose(fp);
}

config_entry_t *parse_config(size_t *count_out)
{
    size_t count = 0;
    size_t capacity = 8;

    config_entry_t *entries = safe_malloc(capacity * sizeof(config_entry_t));

    add_default_entries(&entries, &count, &capacity);
    load_user_config(&entries, &count, &capacity);

    *count_out = count;
    log_message(LOG_INFO, "Loaded %zu total monitored paths (including defaults).", count);

    return entries;
}
