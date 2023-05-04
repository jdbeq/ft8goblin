/*
 * Our overly complicated text based user interface!
 *
 * Widgets Available:
 *	TextArea 	Scrollable text area, wrapped around termbox2 (with autoscroll option)
 *	Menu		Multi-level menus, with history support, similar to WANG mainframes.
 *
 */
#include <errno.h>
#include <termbox2.h>
#include "config.h"
#include "ui.h"
#include "subproc.h"
#include "logger.h"

// Let's not make this public, but rather provide any utilities needed...
static TextArea *ta_textareas[MAX_TEXTAREAS];

////////////////////////////////////////////////////////////
// These let us print unicode to an exact screen location //
// - We call these after clearing screen when scrolling.  //
////////////////////////////////////////////////////////////
void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg) {
   while (*str) {
       uint32_t uni;
       str += tb_utf8_char_to_unicode(&uni, str);
       tb_set_cell(x, y, uni, fg, bg);
       x++;
   }
}

void printf_tb(int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...) {
   char buf[8192];
   va_list vl;
   va_start(vl, fmt);
   vsnprintf(buf, sizeof(buf), fmt, vl);
   va_end(vl);
   print_tb(buf, x, y, fg, bg);
}

//////////////////////////
// Parse out color tags //
//////////////////////////
// XXX: Really we need shorter tokens for these like
static int parse_color(const char *str) {
   int rv = -1;

   if (strncmp(str, "BLACK", 5) == 0) {
      rv = TB_BLACK|TB_BOLD;
   } else if (strncmp(str, "black", 5) == 0) {
      rv = TB_BLACK;
   } else if (strncmp(str, "RED", 3) == 0) {
      rv = TB_RED|TB_BOLD;
   } else if (strncmp(str, "red", 3) == 0) {
      rv = TB_RED;
   } else if (strncmp(str, "GREEN", 5) == 0) {
      rv = TB_GREEN|TB_BOLD;
   } else if (strncmp(str, "green", 5) == 0) {
      rv = TB_GREEN;
   } else if (strncmp(str, "YELLOW", 6) == 0) {
      rv = TB_YELLOW|TB_BOLD;
   } else if (strncmp(str, "yellow", 6) == 0) {
      rv = TB_YELLOW;
   } else if (strncmp(str, "BLUE", 4) == 0) {
      rv = TB_BLUE|TB_BOLD;
   } else if (strncmp(str, "blue", 4) == 0) {
      rv = TB_BLUE;
   } else if (strncmp(str, "MAGENTA", 7) == 0) {
      rv = TB_MAGENTA|TB_BOLD;
   } else if (strncmp(str, "magenta", 7) == 0) {
      rv = TB_MAGENTA;
   } else if (strncmp(str, "CYAN", 4) == 0) {
      rv = TB_CYAN|TB_BOLD;
   } else if (strncmp(str, "cyan", 4) == 0) {
      rv = TB_CYAN;
   } else if (strncmp(str, "WHITE", 5) == 0) {
      rv = TB_WHITE|TB_BOLD;
   } else if (strncmp(str, "white", 5) == 0) {
      rv = TB_WHITE;
   } else
      rv = TB_DEFAULT;

   return rv;
}

typedef struct ColorPair {
   int fg, bg;
} ColorPair;

// Use parse_color above to parse out color strings, whether just $FG$ or $FG,BG$
static ColorPair parse_color_str(const char *str) {
   ColorPair cp;

   return cp;
}

//////////////////////////////////////////////
// Redraw the TextArea from the ring buffer //
//////////////////////////////////////////////
void ta_redraw(TextArea *ta) {
   // Find the end of the ring buffer
   // Start drawing from the bottom up
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

   char *bp = strdup(buf);

   // set needs_freed to ensure it gets freed automatically...
   rb_add(ta->scrollback, bp, true);
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
    // we need to duplicate this and ensure it gets free()'d when bumped out of the ringbuffer...
//    ta_append(ta, buf);
//    print_tb(buf, my_x, my_y, fg, bg);
}

void ui_resize_window(void) {
   height = tb_height();
   width = tb_width();

   if (width < 80 || height < 20) {
      tb_clear();
      tb_present();
      tb_shutdown();
      fprintf(stderr, "[display] Your terminal has a size of %dx%d, this is too small! I cannot continue...\n", width, height);
      dying = 1;
      exit(200);
   } else {
      log_send(mainlog, LOG_NOTICE, "display resolution %dx%d is acceptable!", width, height);
   }
   line_status = height - 1;
   line_input = height - 2;
   ta_resize_all();
   redraw_screen();
}

void ui_shutdown(void) {
   // Tear down to exit
   dying = 1;

   // display a notice that we are exiting and to be patient...
   tb_clear();
   log_send(mainlog, LOG_NOTICE, "ft8goblin exiting, please wait for subprocs to halt...");
//   modal_dialog(0, TB_WHITE|TB_BOLD, TB_RED, TB_BLACK, TB_RED|BOLD, "SHUTTING DOWN", TB_RED|TB_BOLD, TB_BLACK, "ft8goblin is shutting down, please wait...");

   // stop libev stuff...
   // libev_shutdown();

   // send SIGTERM then SIGKILL to subprocesses...
   subproc_shutdown();

   // shut down termbox
   tb_shutdown();
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
      fprintf(stderr, "ui_textarea_init: out of memory!\n");
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
   ta->scrollback = rb_create(ta->scrollback_lines);

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
      ta_redraw(ta_textareas[i]);
   }
}

// send resize all textareas
void ta_resize_all(void) {
   for (int i = 0; i < MAX_TEXTAREAS; i++) {
      ta_resize(ta_textareas[i]);
   }
}

void ui_init(void) {
   tb_init();
}


//////////
// ToDo //
//////////
// Returns: index of button pressed. (Buttons are 0 = OK, 1 = CANCEL, 2 = OK|CANCEL, 3 = OK|CANCEL|HELP)
int modal_dialog(int buttons, int border_fg, int border_bg, int title_fg, int title_bg, const char *title, int text_fg, int text_bg, const char *fmt, ...) {
   va_list ap;

   if (fmt != NULL) {
      va_start(ap, fmt);

      // print the buffer into temporary memory
      char buf[2000];		// an 80x25 screen is 2000 characters..
      memset(buf, 0, 2000);
      vsnprintf(buf, 2000, fmt, ap);
      // we should check return value and errno... ;)

      // XXX: Display into the the modal window
      va_end(ap);
   } else {
      // handle setting up a dialog for "other" uses (not yet supported)
      log_send(mainlog, LOG_CRIT, "modal_dialog doesn't yet support use other than simple alerts. (fmt is NULL). Please fix your code or improve the library!");
      return ENOSYS;
   }
   return 0;
}
