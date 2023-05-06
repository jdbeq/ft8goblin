#include "config.h"
#include "ft8goblin_types.h"
#include "gnis-lookup.h"
#include <sqlite3.h>
#include <spatialite.h>

/*
 * Query the local copy of GNIS database to find a locality near a WGS-84 coordinate
 */
bool gnis_initialized = false;
bool use_gnis = false;
static const char *gnis_db = NULL;

int gnis_init(void) {
   const char *s = cfg_get_str(cfg, "gnis-lookup/use-gnis");

   if (s != NULL && strncasecmp(s, "true", 4) == 0) {
      use_gnis = true;
   } else {
      use_gnis = false;
   }

   s = cfg_get_str(cfg, "gnis-lookup/gnis-db");
   if (s != NULL) {
      gnis_db = s;
   }

   gnis_initialized = true;

   return 0;
}
