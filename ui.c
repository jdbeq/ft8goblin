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

// main msg area
TextArea *msgbox = NULL;

// XXX: These should all go into a ConfigData struct
const char *mycall = NULL;	// cfg:ui/mycall
const char *gridsquare = NULL;	// cfg:ui/gridsquare

////////////////
int	line_status = -1;		// status line
int 	line_input = -1;		// input field
int	height = -1, width = -1;
int	active_band = 40;		// Which band are we TXing on?
int	active_pane = 1;		// active pane (0: TextArea, 1: TX input)

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

///////////////////////////////////////////////////////
static void print_help(void) {
   int offset = 0;
   printf_tb(offset, 0, TB_GREEN|TB_BOLD, 0, "*Keys* ");
   offset += 7;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "ESC ");
   offset += 4;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Go Back ");
   offset += 8;

   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "TAB ");
   offset += 4;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Switch Panes ");
   offset += 13;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "^Q/^X ");
   offset += 6;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Exit ");
   offset += 5;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "^B ");
   offset += 3;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Band Menu ");
   offset += 10;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "^M ");
   offset += 3;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Menu ");
   offset += 5;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "^T ");
   offset += 3;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Toggle TX ");
   offset += 10;
   printf_tb(offset, 0, TB_RED|TB_BOLD, 0, "^H ");
   offset += 3;
   printf_tb(offset, 0, TB_MAGENTA|TB_BOLD, 0, "Halt TX immed.");
}

static void print_status(void) {
   int offset = 0;

   // callsign
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "[MyCall:");
   offset += 8;
   printf_tb(offset, height - 1, TB_CYAN|TB_BOLD, 0, "%s", mycall);
   offset += strlen(mycall);
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "] ");
   offset += 2;

   // grid square
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "[MyGrid:");
   offset += 8;
   printf_tb(offset, height - 1, TB_CYAN|TB_BOLD, 0, "%s", gridsquare);
   offset += strlen(gridsquare);
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "] ");
   offset += 2;

   // TX enabled status
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "[");
   offset++;
   printf_tb(offset, height - 1, TB_GREEN|TB_BOLD, 0, "TX:");
   offset += 3;

   if (tx_enabled) {
      printf_tb(offset, height - 1, TB_RED|TB_BOLD, 0, "ON");
      offset += 2;
   } else {
      printf_tb(offset, height - 1, TB_GREEN|TB_BOLD, 0, "OFF");
      offset += 3;
   }
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "] ");
   offset += 2;

   // show bands with TX enabled, from yajl tree...
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "[");
   offset++;
   printf_tb(offset, height - 1, TB_GREEN|TB_BOLD, 0, "TXBand:");
   offset += 7;

   if (active_band != 0) {
      printf_tb(offset, height - 1, TB_RED|TB_BOLD, 0, "%dm", active_band);
      offset += 3;
   }

   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "] ");
   offset += 2;

   
   // print the PTT status
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "[");
   offset++;
   printf_tb(offset, height - 1, TB_GREEN|TB_BOLD, 0, "PTT:");
   offset += 4;
#if	0
   // Explode the list of radios actively PTTing
   for (int i = 0; i < max_rigs; i++) {
      if (rigs[i].ptt_active) {
         printf_tb(offset, height - 1, TB_RED|TB_BOLD, 0, "%d", i);
      }
   }
#endif
   printf_tb(offset, height - 1, TB_WHITE|TB_BOLD, 0, "] ");
   offset += 2;
   tb_present();
}

static void print_input(void) {
   // XXX: Draw the input text area
}

void redraw_screen(void) {
   print_help();
   ta_redraw(msgbox);
   print_input();
   print_status();
   tb_present();
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
      ta_printf(msgbox, "$WHITE$[$GREEN$display$WHITE$] Resolution %dx%d is acceptable!", width, height);
   }

// XXX: Need a way to call this automatically for all TextAreas?
   msgbox->top = 1;
   msgbox->bottom = height - 3;

   line_status = height - 1;
   line_input = height - 2;
}

void ui_shutdown(void) {
   // Tear down to exit
   dying = 1;

   // display a notice that we are exiting and to be patient...
   tb_clear();
   ta_printf(msgbox, "$RED$ft8goblin exiting, please wait for subpprocesses to halt...");
   tb_present();

   // stop libev stuff...
   // libev_shutdown();

   // send SIGTERM then SIGKILL to subprocesses...
   subproc_shutdown();

   // shut down termbox
   tb_shutdown();

   printf("ft8goblin exited cleanly!\n");
   exit(0);
}

void ta_destroy(TextArea *ta) {
   if (ta == NULL) {
      return;
   }

   if (ta->scrollback != NULL) {
      rb_destroy(ta->scrollback);
      ta->scrollback = NULL;
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
      ta->scrollback_lines = tb_height() * 5;
      printf("ui/scrollback-lines not set or unparsable in configuration. defaulting to %d", ta->scrollback_lines);
   }
   ta->scrollback = rb_create(ta->scrollback_lines);

   return ta;
}

void ui_init(void) {
   tb_init();
   msgbox = ta_init(cfg_get_int(cfg, "ui/scrollback-lines"));
}
