#include "../include/logging.h"

static FILE           *log_fp = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_init(const char *ident, int option, int facility)
{
    openlog(ident, option, facility);

    log_fp = fopen(LOG_FILE, "a");
    if (!log_fp)
        syslog(LOG_ERR, "Failed to open log file: %s", LOG_FILE);

    log_message(LOG_INFO, "Starting %s...", ident);
}

void log_close(void)
{
    if (log_fp)
        fclose(log_fp);
    closelog();
}

static void get_timestamp(char *buf, size_t size)
{
    time_t     now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, size, "[%Y-%m-%d %H:%M:%S]", tm_info);
}

void log_message(int priority, const char *fmt, ...)
{
    va_list args;

    pthread_mutex_lock(&log_mutex);

    va_start(args, fmt);
    vsyslog(priority, fmt, args);
    va_end(args);

    if (log_fp)
    {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));

        va_start(args, fmt);
        fprintf(log_fp, "%s ", timestamp);
        vfprintf(log_fp, fmt, args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
        va_end(args);
    }

    pthread_mutex_unlock(&log_mutex);
}
