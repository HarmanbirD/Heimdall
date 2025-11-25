#ifndef HEIMDALL_DATABASE_H
#define HEIMDALL_DATABASE_H

#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>

struct log_state;

typedef struct db_state
{
    sqlite3        *handle;
    pthread_mutex_t mutex;
    int             mutex_initialized;
} db_state;

int db_init(struct db_state *db, const char *path, struct log_state *ls);
int db_close(struct db_state *db);
int database_record_hash(struct db_state  *db,
                         struct log_state *logger,
                         uint32_t          client_id,
                         const char       *client_ip,
                         const char       *hostname,
                         const char       *file_path,
                         uint64_t          timestamp,
                         const char       *hash_hex);

#endif // HEIMDALL_DATABASE_H
