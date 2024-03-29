/*
 * Utility functions to make working with SQL more pleasant
 */
#include "config.h"
#include "ft8goblin_types.h"
#include "sql.h"

// Include sqlite3 and spatialite extensions as they are our default database for read-only info
#include <sqlite3.h>
#include <spatialite.h>

// Include libpq for postgresql
#if	defined(USE_POSTGRESQL)
#include <postgresql/libpq-fe.h>
#endif

// Include mysql support
#if	defined(USE_MYSQL)
#endif

typedef struct Database {
   enum {
      DB_NONE = 0,
      DB_SQLITE3,
      DB_POSTGRES,
      DB_MYSQL
   } db_type;

   // 
} Database;

Database *sql_open(const char *path) {
    Database *db = NULL;

    if (path == NULL) {
       fprintf(stderr, "sql_open: path is NULL!\n");;
       return NULL;
    }

    if ((db = malloc(sizeof(Database))) == NULL) {
       fprintf(stderr, "sql_open: out of memory!\n");
       exit(ENOMEM);
    }

    // Is this sqlite3?
    if ((strncasecmp(path, "sqlite:", 7) == 0) || (strncasecmp(path, "sqlite3:", 8) == 0)) {
       // open the sqlite database
       db->db_type = DB_SQLITE3;
    } else if ((strncasecmp(path, "postgres:", 8) == 0) || (strncasecmp(path, "pgsql:", 6) == 0)) {
       // open postgresql database
       db->db_type = DB_POSTGRES;
    } else if (strncasecmp(path, "mysql:", 6) == 0) {
       // XXX: Someone should write mysql support
       db->db_type = DB_MYSQL;
    }

    return db;
}

int	sql_close(void) {
    return 0;
}
