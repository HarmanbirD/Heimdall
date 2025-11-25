#include "daemonize_control.h"
#include "config.h"
#include "daemonize.h"
#include "logging.h"
#include <sys/types.h>
#include <unistd.h>

bool is_running_under_systemd(void)
{
    return (getppid() == 1);
}

void maybe_daemonize(void)
{
    if (!is_running_under_systemd())
    {
        log_message(LOG_INFO, "Not running under systemd. Daemonizing manually...");

        start_daemon(HEIMDALL_WORKDIR, HEIMDALL_UMASK, HEIMDALL_PIDFILE);
    }
    else
    {
        log_message(LOG_INFO, "Running under systemd. Skipping manual daemonization.");
    }
}
