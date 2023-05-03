/*
 * Our overly complicated text based user interface!
 *
 * Widgets Available:
 *	TextArea 	Scrollable text area, wrapped around termbox2 (with autoscroll option)
 *	Menu		Multi-level menus, with history support, similar to WANG mainframes.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ev.h>
#include <evutil.h>
#include "config.h"
#include "subproc.h"
#include "util.h"
#include <termbox2.h>
#include "ui.h"

/////////////////////////////////////////
int	dying = 0;		// Are we shutting down?
int	tx_enabled = 0;		// Master toggle to TX mode.
int	scrollback_lines = -1;	// this is set in main below...
int	active_band = 40;	// Which band are we TXing on?
int	active_pane = 0;	// active pane (0: TextArea, 1: TX input)

int height = -1, width = -1;
int line_textarea_top = -1,	// top of scrollable TextArea
    line_textarea_bottom = -1;	// bottom of scrollable TextArea
int line_status = -1;		// status line
int line_input = -1;		// input field

///////////////////////////////////
// XXX: These belong in config.c //
///////////////////////////////////
const char *mycall = NULL;	// cfg:ui/mycall
const char *gridsquare = NULL;	// cfg:ui/gridsquare

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
void ta_redraw(void) {
   // Find the end of the ring buffer
   // Start drawing from the bottom up
   for (int i = line_textarea_bottom; i--; i > line_textarea_top) {
      // Draw the current line of the ring buffer

      // Is the previous line valid?

      // If not, break
//      break;
   }
}

//////////////////////////////////////////
// Append to the end of the ring buffer //
//////////////////////////////////////////
int ta_append(const char *buf) {
   int rv = 0;

   // Find end of ring buffer (last timestamp)
   // Replace the NEXT entry in ring buffer with our buffer
   return rv;
}
///////////////////////////
// Print to the TextArea //
///////////////////////////
void ta_printf(const char *fmt, ...) {
    char buf[4096];
    int bg = 0, fg = 0, my_y = 0, my_x = 0;
    va_list vl;
    va_start(vl, fmt);

    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    ta_append(buf);
//    print_tb(buf, my_x, my_y, fg, bg);
}

///////////////////////////////////////////////////////
static void exit_fix_config(void) {
   printf("Please edit your config.json and try again!\n");
   exit(255);
}

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
   // XXX: do this
   ta_redraw();
   print_input();
   print_status();
   tb_present();
}

//////////////////////////////////////////
// Handle termbox tty and resize events //
//////////////////////////////////////////
static void termbox_cb(EV_P_ ev_timer *w, int revents) {
   struct tb_event evt;		// termbox io events
   tb_poll_event(&evt);
   process_input(&evt);
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
      ta_printf("$WHITE$[$GREEN$display$WHITE$] Resolution %dx%d is acceptable!", width, height);
   }

   line_textarea_top = 1;
   line_textarea_bottom = height - 3;
   line_status = height - 1;
   line_input = height - 2;
   scrollback_lines = tb_height() * 5;
}

void ui_shutdown(void) {
   // Tear down to exit
   dying = 1;

   // display a notice that we are exiting and to be patient...
   tb_clear();
   ta_printf("$RED$ft8goblin exiting, please wait for subpprocesses to halt...");
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

int main(int argc, char **argv) {
   int fd_tb_tty = -1, fd_tb_resize = -1;
   ev_io termbox_watcher, termbox_resize_watcher;
   struct ev_loop *loop = EV_DEFAULT;

   // print this even though noone will see it, except in case of error exit ;)
   printf("ft8goblin: A console based ft8 client with support for multiband operation\n\n");

   // This can't work without a valid configuration...
   if (!(cfg = load_config()))
      exit_fix_config();

   mycall = cfg_get_str(cfg, "site/mycall");
   gridsquare = cfg_get_str(cfg, "site/gridsquare");

   ///////////////////////////
   // Perform startup tasks //
   ///////////////////////////

   // Initialize termbox
   tb_init();
   ui_resize_window();
   ta_printf("$CYAN$Welcome to ft8goblin, a console ft8 client with support for multiple bands!");

   /////////////////////////////////
   // Setup the scrollback buffer //
   /////////////////////////////////
   scrollback_lines = cfg_get_int(cfg, "ui/scrollback-lines");

   if (scrollback_lines == -1) {
      scrollback_lines = tb_height() * 5;
      printf("ui/scrollback-lines not set or unparsable in configuration. defaulting to %d", scrollback_lines);
   }

   /////////////////////
   // Draw the screen //
   /////////////////////
   redraw_screen();

   ///////////////////////////////////////////
   // Setup libev to handle termbox2 events //
   ///////////////////////////////////////////
   tb_get_fds(&fd_tb_tty, &fd_tb_resize);

   // stdio occupy 0-2 (stdin, stdout, stderr)
   if (fd_tb_tty >= 2 && fd_tb_resize >= 2) {
      // add to libev set
   } else {
      ta_printf("$RED$tb_get_fds returned nonsense (%d, %d) can't continue!", fd_tb_tty, fd_tb_resize);
      tb_present();
      exit(200);
   }
   ev_io_init(&termbox_watcher, termbox_cb, fd_tb_tty, EV_READ);
   ev_io_init(&termbox_resize_watcher, termbox_cb, fd_tb_resize, EV_READ);
   ev_io_start(loop, &termbox_watcher);
   ev_io_start(loop, &termbox_resize_watcher);

   // Initialize the GNIS place names database

   // Initialize the Callsign lookup system

   // Load the watchlists
   watchlist_load(cfg_get_str(cfg, "ui/alerts/watchfile"));

   // Set up IPC

   // Start supervising subprocesses:
   //	ft8capture (single instance per device)
        // XXX: Walk the tree at cfg:devices
//        subproc_start()
        // XXX: Walk the tree at cfg:bands
   //	ft8decoder (one per band)

   // main loop...
   while (!dying) {
      // XXX: do libev stuff and decide if there's waiting input on fd_tb_tty or fd_fb_resize and act appropriately...
      ev_run (loop, 0);

      // if ev loop exits, we need to die..
      dying = 1;
   }

   ui_shutdown();
   return 0;
}

///////////
// Menus //
///////////
int view_config(void) {
   // XXX: Show the yajl config tree as a scrollable 'menu', without editing
   return 0;
}

void halt_tx_now(void) {
   ta_printf("$RED$Halting TX!");
   redraw_screen();
   tb_present();
}
