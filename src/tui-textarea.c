/*
 * tui TextArea widget
 *
 * This code probably isn't very reusable yet. That's being worked on...
 */
#include "config.h"
#include "debuglog.h"
#include "tui.h"
#include "subproc.h"
#include "ft8goblin_types.h"
#include <errno.h>
#include <termbox2.h>
extern TextArea *msgbox;

// Let's not make this public, but rather provide any utilities needed...
static TextArea *ta_textareas[MAX_TEXTAREAS];

//////////////////////////////////////////////
// Redraw the TextArea from the ring buffer //
//////////////////////////////////////////////
void ta_redraw(TextArea *ta) {
   // Find the end of the ring buffer
   // Start drawing from the bottom up
   // XXX: Make this work
   if (ta->scrollback == NULL) {
      return;
   }
   rb_node_t *rbn = ta->scrollback->head;
   rb_node_t *latest_rn = rbn;

   do {
      if (rbn == NULL) {
         break;
      }

      if (rbn->timestamp.tv_sec > latest_rn->timestamp.tv_sec) {
         latest_rn = rbn;
      }

      if (rbn->next == NULL) {
         break;
      }
      rbn = rbn->next;
   } while (true);

   void *sb = rb_get_most_recent(ta->scrollback);
   for (int i = ta->bottom; i > ta->top; i--) {
      // Draw the current line of the ring buffer

      // Is the previous line valid?

      // If not, break
//      break;
   }
}

void ta_resize(TextArea *ta) {
   // XXX: We have to calculate sizes, if this is to be used for multiple TextAreas....
}

//////////////////////////////////////////
// Append to the end of the ring buffer //
//////////////////////////////////////////
int ta_append(TextArea *ta, const char *buf) {
   int rv = 0;

   if (ta == NULL || buf == NULL) {
      return -1;
   }

   // duplicate the buffer
   char *bp = strdup(buf);

   // set needs_freed to ensure it gets freed automatically...
   rb_add(ta->scrollback, bp, true);

   // XXX: temporarily send TextArea messages to log (yuck!)
   log_send(mainlog, LOG_DEBUG, "%s", buf);
   return rv;
}

///////////////////////////
// Print to the TextArea //
///////////////////////////
void ta_printf(TextArea *ta, const char *fmt, ...) {
    if (ta == NULL || fmt == NULL)
       return;

    char buf[4096];
    int bg = 0, fg = 0;
    va_list vl;
    va_start(vl, fmt);

    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    ta_append(ta, buf);
}

void ta_destroy(TextArea *ta) {
   if (ta == NULL) {
      return;
   }

   if (ta->scrollback != NULL) {
      rb_destroy(ta->scrollback);
      ta->scrollback = NULL;
   }

   for (int i = 0; i < MAX_TEXTAREAS; i++) {
      if (ta_textareas[i] == ta) {
         ta_textareas[i] = NULL;
         i++;
         // Work backwards from the end, bumping all the entries down one..
         for (int j = MAX_TEXTAREAS; j < i; j--) {
            log_send(mainlog, LOG_DEBUG, "Moving ta_textareas slot %d to slot %i", j, j - 1);
            ta_textareas[j - 1] = ta_textareas[j];
         }
         break;
      }
   }
   free(ta);
}

TextArea *ta_init(int scrollback_lines) {
   TextArea *ta = NULL;

   if ((ta = malloc(sizeof(TextArea))) == NULL) {
      fprintf(stderr, "ta_init: out of memory!\n");
      exit(ENOMEM);
   }

   /////////////////////////////////
   // Setup the scrollback buffer //
   /////////////////////////////////
   ta->scrollback_lines = scrollback_lines;

   if (ta->scrollback_lines == -1) {
      log_send(mainlog, LOG_CRIT, "ta_init: refusing to create TextArea with no scrollback as this can't hold text!");
      return NULL;
   }
   ta->scrollback = rb_create(ta->scrollback_lines, "TextArea scrollback");

   // add to end of ta_textareas array
   for (int i = 0; i < MAX_TEXTAREAS; i++) {
      if (ta_textareas[i] == NULL) {
         ta_textareas[i] = ta;
         break;
      }
   }
   return ta;
}

// redraw *ALL* textareas
void ta_redraw_all(void) {
   for (int i = 0; i < MAX_TEXTAREAS; i++) {
      if (ta_textareas[i] == NULL)
         continue;
      ta_redraw(ta_textareas[i]);
   }
}

// send resize all textareas
void ta_resize_all(void) {
   for (int i = 0; i < MAX_TEXTAREAS; i++) {
      if (ta_textareas[i] == NULL)
         continue;
      ta_resize(ta_textareas[i]);
   }
}
