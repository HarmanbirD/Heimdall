#ifndef LOGGING_H
#define LOGGING_H

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

#define LOG_FILE "/var/log/heimdall.log"

void log_init(const char *ident, int option, int facility);
void log_message(int priority, const char *fmt, ...);
void log_close(void);

#endif // LOGGING_H
