#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <sys/types.h>

void daemonize(const char *workdir, mode_t umask_val, const char *pidfile);
int  create_pidfile(const char *pidfile);
int  start_daemon(const char *workdir, mode_t umask_val, const char *pidfile);

#endif // DAEMONIZE_H
