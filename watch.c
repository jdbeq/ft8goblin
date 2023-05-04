#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include "config.h"
#include "watch.h"

// Destroy the watch pointer w.
// After this function returns ESUCCESS (0), all pointers
// are to the watch_item_t at w are no longer valid and
// should be set to NULL.
int watch_destroy(watch_item_t *w) {
    if (w == NULL) {
       fprintf(stderr, "watch_destroy called with w = NULL!\n");
       return -EFAULT;
    }

    if (w->watch_string_sz > 0) {
       if (w->watch_string != NULL) {
          free(w->watch_string);
          w->watch_string = NULL;
          w->watch_string_sz = 0;
       } else {
          // XXX: This shouldn't happen...
          fprintf(stderr, "watch_destroy called on w <%p> with watch_string_sz %lu but watch_string is NULL!\n", (void *)w, w->watch_string_sz);
          return -EFAULT;
       }
    } else {
       // Catch some error conditions...
       if (w->watch_string != NULL) {
          fprintf(stderr, "watch_destroy: w <%p>'s watch_string_sz %lu <= 0 yet watch_string is not NULL <%s>!\n", (void *)w, w->watch_string_sz, w->watch_string);
          return -EFAULT;
       }
    }
    free(w);
    return 0;
}

watch_item_t *watch_create(watch_type_t watch_type, const char *watch_string, size_t watch_string_sz, int watch_regex_level) {
    watch_item_t *w = NULL;

    if (watch_type == WATCH_NONE || watch_string == NULL || watch_string_sz <= 0 || watch_regex_level < 0) {
       fprintf(stderr, "watch_create called with invalid parameters: watch_type %i, watch_string <%s>, watch_string_sz %lu, watch_regex_level %i\n", watch_type, watch_string, watch_string_sz, watch_regex_level);
       return NULL;
    }

    // error handling
    if ((w = (watch_item_t *)malloc(sizeof(watch_item_t))) == NULL) {
       fprintf(stderr, "watch_create failed to allocate watch_item_t. out of memory, exiting!\n");
       exit(255);
    }

    // store the data
    w->watch_type = watch_type;
    w->watch_string_sz = watch_string_sz;
    w->watch_string = malloc(w->watch_string_sz);
    w->watch_regex_level = watch_regex_level;

    // return the item
    return w;
}

//////////////////////////////////
// Load a watchlist into memory //
//////////////////////////////////
int watchlist_load(const char *path) {
   FILE *fp = NULL;

   if ((fp = fopen(path, "r")) == NULL) {
      fprintf(stderr, "watchlist_loads: Failed opening %s: %d:%s\n", path, errno, strerror(errno));
      return -errno;
   }

   while (!feof(fp)) {
      //
      break;
   }

   fclose(fp);
   fp = NULL;
   return 0;
}
