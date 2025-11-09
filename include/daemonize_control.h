#ifndef HEIMDALL_DAEMONIZE_CONTROL_H
#define HEIMDALL_DAEMONIZE_CONTROL_H

#include <stdbool.h>

bool is_running_under_systemd(void);
void maybe_daemonize(void);

#endif
