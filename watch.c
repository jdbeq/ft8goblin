#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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
          fprintf(stderr, "watch_destroy called on w <%x> with watch_string_sz <%d> but watch_string is NULL!\n", w, w->watch_string_sz);
          return -EFAULT;
       }
    } else {
       // Catch some error conditions...
       if (w->watch_string != NULL) {
          fprintf(stderr, "watch_destroy: w <%x>'s watch_string_sz <%d> <= 0 yet watch_string is not NULL <%x>!\n", w, w->watch_string_sz, w->watch_string);
          return -EFAULT;
       }
    }
    free(w);
    return 0;
}

watch_item_t *watch_create(watch_type_t watch_type, const char *watch_string, size_t watch_string_sz, int watch_regex_level) {
    watch_item_t *w = NULL;

    if (watch_type == WATCH_NONE || watch_string == NULL || watch_string_sz <= 0 || watch_regex_level < 0) {
       fprintf(stderr, "watch_create called with invalid parameters: watch_type <%d>, watch_string <%s>, watch_string_sz <%d>, watch_regex_level <%d>\n", watch_type, watch_string, watch_string_sz, watch_regex_level);
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
