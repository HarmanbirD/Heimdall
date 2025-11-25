#include "database.h"

#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

static int exec_pragma(sqlite3 *db, const char *pragma)
{
    char *err_msg = NULL;

    int rc;
    rc = sqlite3_exec(db, pragma, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(err_msg);
    }

    return rc;
}

static int create_schema(sqlite3 *db)
{
    const char *schema_sql =
        "CREATE TABLE IF NOT EXISTS clients ("
        "id INTEGER PRIMARY KEY,"
        "ip_address TEXT,"
        "hostname TEXT,"
        "yellow_interval INTEGER"
        ");"
        "CREATE TABLE IF NOT EXISTS files ("
        "id INTEGER PRIMARY KEY,"
        "client_id INTEGER,"
        "path TEXT,"
        "mode INTEGER,"
        "FOREIGN KEY(client_id) REFERENCES clients(id) ON DELETE CASCADE"
        ");"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_files_client_path ON files(client_id, path);"
        "CREATE TABLE IF NOT EXISTS hash_records ("
        "id INTEGER PRIMARY KEY,"
        "file_id INTEGER,"
        "timestamp INTEGER,"
        "hash TEXT,"
        "FOREIGN KEY(file_id) REFERENCES files(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS alerts ("
        "id INTEGER PRIMARY KEY,"
        "file_id INTEGER,"
        "timestamp INTEGER,"
        "level INTEGER,"
        "message TEXT,"
        "FOREIGN KEY(file_id) REFERENCES files(id) ON DELETE CASCADE"
        ");";

    char *err_msg = NULL;

    int rc;
    rc = sqlite3_exec(db, schema_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(err_msg);
    }

    return rc;
}

int db_init(struct db_state *db, const char *path, struct log_state *ls)
{
    if (!db || !path)
        return -1;

    memset(db, 0, sizeof(*db));

    int rc;
    rc = sqlite3_open(path, &db->handle);
    if (rc != SQLITE_OK)
    {
        logging_log(ls, LOG_ERR, "Failed to open database: %s", sqlite3_errmsg(db->handle));
        db_close(db);

        return -1;
    }

    sqlite3_busy_timeout(db->handle, 5000);

    if (exec_pragma(db->handle, "PRAGMA foreign_keys = ON;") != SQLITE_OK ||
        exec_pragma(db->handle, "PRAGMA journal_mode = WALL;") != SQLITE_OK ||
        exec_pragma(db->handle, "PRAGMA synchronous = FULL;") != SQLITE_OK)
    {
        logging_log(ls, LOG_ERR, "Failed to apply PRAGMA to settings.");
        db_close(db);

        return -1;
    }

    if (create_schema(db->handle) != SQLITE_OK)
    {
        logging_log(ls, LOG_ERR, "Failed to create schema: %s", sqlite3_errmsg(db->handle));
        db_close(db);

        return -1;
    }

    if (!db->mutex_initialized)
    {
        pthread_mutex_init(&db->mutex, NULL);
        db->mutex_initialized++;
    }

    return 0;
}

static int begin_transaction(sqlite3 *db)
{
    return sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, NULL);
}

static int commit_transaction(sqlite3 *db)
{
    return sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
}

static int rollback_transaction(sqlite3 *db)
{
    return sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
}

int database_record_hash(struct db_state  *db,
                         struct log_state *logger,
                         uint32_t          client_id,
                         const char       *client_ip,
                         const char       *hostname,
                         const char       *file_path,
                         uint64_t          timestamp,
                         const char       *hash_hex)
{

    return 0;
}

int db_close(struct db_state *db)
{
    if (!db || !db->handle)
        return -1;

    if (db->mutex_initialized)
        pthread_mutex_destroy(&db->mutex);

    sqlite3_close(db->handle);
    db->handle = NULL;

    return 0;
}
