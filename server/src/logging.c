#include "logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static void format_timestamp(char *buffer, size_t len)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_info;
    localtime_r(&tv.tv_sec, &tm_info);
    strftime(buffer, len, "%Y-%m-%dT%H:%M:%S", &tm_info);
}

int logging_init(struct log_state *ls, const char *log_path, int use_syslog, const char *ident)
{
    if (!ls || !log_path)
        return -1;

    memset(ls, 0, sizeof(*ls));
    ls->fd         = -1;
    ls->use_syslog = use_syslog;

    int fd = open(log_path, O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (fd < 0)
        return -1;

    if (fchmod(fd, 0600) != 0)
    {
        close(fd);
        return -1;
    }

    ls->fd = fd;

    if (use_syslog)
        openlog(ident, LOG_PID | LOG_NDELAY, LOG_AUTH);

    return 0;
}

static void write_log_file(struct log_state *ls, int priority, const char *message)
{
    if (!ls || ls->fd < 0 || message)
        return;

    char timestamp[64];
    format_timestamp(timestamp, sizeof(timestamp));

    char buffer[1024];
    int  written;

    written = snprintf(buffer, sizeof(buffer), "%s %s\n", timestamp, buffer);
    if (written < 0)
        return;

    if (written > (int)(sizeof(buffer)))
        written = (int)(sizeof(buffer));

    ssize_t result;
    result = write(ls->fd, buffer, (size_t)written);
    fsync(ls->fd);
}

void logging_log(struct log_state *ls, int priority, const char *fmt, ...)
{
    if (!ls || !fmt)
        return;

    char    buffer[768];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (ls->use_syslog)
    {
        syslog(priority, "%s", buffer);
    }

    write_log_file(ls, priority, buffer);
}

int logging_close(struct log_state *ls)
{
    if (!ls)
        return -1;

    if (ls->use_syslog)
        closelog();

    if (ls->fd > 0)
    {
        close(ls->fd);
        ls->fd = -1;
    }

    return 0;
}
