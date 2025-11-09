#include "../include/daemonize.h"
#include "../include/logging.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

static char pidfile_path[PATH_MAX] = {0};

static pid_t safe_fork(void)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        log_message(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS);

    return pid;
}

static void cleanup_and_exit(int sig)
{
    log_message(LOG_INFO, "Received signal %d, cleaning up...", sig);

    if (pidfile_path[0] != '\0')
    {
        if (remove(pidfile_path) == 0)
            log_message(LOG_INFO, "Removed PID file: %s", pidfile_path);
        else
            log_message(LOG_WARNING, "Failed to remove PID file: %s", pidfile_path);
    }

    log_message(LOG_INFO, "Heimdall shutdown.");
    log_close();

    exit(EXIT_SUCCESS);
}

void daemonize(const char *workdir, mode_t umask_val, const char *pidfile)
{
    pid_t pid;

    pid = safe_fork();

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGHUP, SIG_IGN);

    pid = safe_fork();
    (void)pid;

    umask(umask_val);
    chdir(workdir ? workdir : "/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (pidfile)
        strncpy(pidfile_path, pidfile, sizeof(pidfile_path) - 1);

    signal(SIGTERM, cleanup_and_exit);
    signal(SIGINT, cleanup_and_exit);
    signal(SIGQUIT, cleanup_and_exit);
}

int create_pidfile(const char *pidfile)
{
    FILE *fp;
    pid_t pid;

    fp = fopen(pidfile, "r");
    if (fp)
    {
        if (fscanf(fp, "%d", &pid) == 1)
        {
            if (kill(pid, 0) == 0)
            {
                log_message(LOG_ERR, "Another instance (PID %d) is already running.", pid);
                fclose(fp);
                return -1;
            }
            else
            {
                log_message(LOG_WARNING, "Stale PID file found (PID %d not running).", pid);
            }
        }
        fclose(fp);
    }

    fp = fopen(pidfile, "w");
    if (!fp)
    {
        log_message(LOG_ERR, "Failed to write PID file %s: %s", pidfile, strerror(errno));
        return -1;
    }

    fprintf(fp, "%d\n", getpid());
    fclose(fp);
    return 0;
}

int start_daemon(const char *workdir, mode_t umask_val, const char *pidfile)
{
    daemonize(workdir, umask_val, pidfile);

    if (create_pidfile(pidfile) != 0)
    {
        log_message(LOG_ERR, "Failed to create PID file: %s", pidfile);

        return -1;
    }

    log_message(LOG_INFO, "Daemon started successfully (PID file: %s)", pidfile);

    return 0;
}
