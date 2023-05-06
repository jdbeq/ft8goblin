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
#include <stdbool.h>
#include <stdint.h>

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
