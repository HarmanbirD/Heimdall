#ifndef HEIMDALL_LOGGING_H
#define HEIMDALL_LOGGING_H

#include <stdarg.h>
#include <syslog.h>

typedef struct log_state
{
    int fd;
    int use_syslog;
} log_state;

int  logging_init(struct log_state *ls, const char *log_path, int use_syslog, const char *ident);
int  logging_close(struct log_state *ls);
void logging_log(struct log_state *ls, int priority, const char *fmt, ...);

#endif // HEIMDALL_LOGGING_H
