/*
 * Support for looking up callsigns using FCC ULS (local) or QRZ XML API (paid)
 *
 * XXX: Add caching support for online lookups, so they can be limited to once per day.
 */

// XXX: Here we need to try looking things up in the following order:
//	Cache
//	FCC ULS Database
//	QRZ XML API
//
// We then need to save it to the cache (if it didn't come from there already)
#include "config.h"
#include "qrz-xml.h"
#include "debuglog.h"
#include <stdbool.h>
#include <stdint.h>

static bool callsign_use_uls = false, callsign_use_qrz = false, callsign_initialized = false;

void callsign_setup(void) {
   callsign_initialized = true;

   // Use local ULS database?
   const char *s = cfg_get_str(cfg, "callsign-lookup/use-uls");

   if (strncasecmp(s, "true", 4) == 0) {
      callsign_use_uls = true;
   } else {
      callsign_use_uls = false;
   }

   // use QRZ XML API?
   s = cfg_get_str(cfg, "callsign-lookup/use-qrz");

   if (strncasecmp(s, "true", 4) == 0) {
      callsign_use_qrz = true;
   } else {
      callsign_use_qrz = false;
   }
}
   
// save a callsign record to the cache
bool callsign_cache_save(qrz_callsign_t *cp) {
    return false;
}

qrz_callsign_t *callsign_cache_find(const char *callsign) {
    return NULL;
}

qrz_callsign_t *callsign_lookup(const char *callsign) {
    bool from_cache = false;
    void *lp = NULL;		// lookup pointer

    if (!callsign_initialized) {
       callsign_setup();
    }

    // Look in cache
    qrz_callsign_t *cp = NULL;
    if ((cp = callsign_cache_find(callsign)) != NULL) {
       from_cache = true;
       return cp;
    }

    // XXX: nope, check FCC ULS

    // XXX: nope, check QRZ XML API

    // only save it in cache if it did not come from there already
    if (!from_cache) {
       callsign_cache_save(cp);
       return cp;
    }

    return NULL;
}
