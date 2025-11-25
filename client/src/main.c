#include "config.h"
#include "daemonize.h"
#include "daemonize_control.h"
#include "hashing.h"
#include "logging.h"
#include "utils.h"
#include <unistd.h>

int main(void)
{
    log_init(LOG_IDENT, LOG_PID, LOG_DAEMON);
    maybe_daemonize();

    while (1)
    {
        log_message(LOG_INFO, "Heimdall: Performing integrity check...");

        integrity_check();

        sleep(FIM_INTERVAL_SEC);
    }

    log_message(LOG_INFO, "Heimdall shutting down cleanly.");
    log_close();

    return EXIT_SUCCESS;
}
