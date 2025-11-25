#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>

#define LOG_IDENT "Heimdall"
#define LOG_FACILITY LOG_DAEMON
#define LOG_FILE "/var/log/heimdall.log"

#define HEIMDALL_PIDFILE "/var/run/heimdall.pid"
#define HEIMDALL_WORKDIR "/"
#define HEIMDALL_UMASK 0

#define FIM_INTERVAL_SEC 120

#define CONFIG_PATH "/etc/heimdall.conf"

#define HASH_HEX_LEN (SHA256_DIGEST_LENGTH * 2)

#endif // CONFIG_H
