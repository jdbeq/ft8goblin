#include "config.h"
#include "subproc.h"
#include "util.h"
#include "ui.h"
#include "logger.h"
#include "watch.h"
#include "daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ev.h>
#include <evutil.h>
#include <termbox2.h>

char *progname = "ft8goblin";
TextArea *msgbox = NULL;
const char *mycall = NULL;	// cfg:ui/mycall
const char *gridsquare = NULL;	// cfg:ui/gridsquare
int	dying = 0;		// Are we shutting down?
int	tx_enabled = 0;		// Master toggle to TX mode.

int	line_status = -1;		// status line
int 	line_input = -1;		// input field
int	height = -1, width = -1;
int	active_band = 40;		// Which band are we TXing on?
int	active_pane = 1;		// active pane (0: TextArea, 1: TX input)

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
   ta_redraw_all();
   print_input();
   print_status();
   tb_present();
}

int main(int argc, char **argv) {
   struct ev_loop *loop = EV_DEFAULT;

   // print this even though noone will see it, except in case of error exit ;)
   printf("ft8goblin: A console based ft8 client with support for multiband operation\n\n");

   // This can't work without a valid configuration...
   if (!(cfg = load_config()))
      exit_fix_config();

   mycall = cfg_get_str(cfg, "site/mycall");
   gridsquare = cfg_get_str(cfg, "site/gridsquare");

   mainlog = log_open(dict_get(runtime_cfg, "logpath", "file://ft8goblin.log.txt"));

   ui_init();
   // create the default TextArea for messages
   msgbox = ta_init(cfg_get_int(cfg, "ui/scrollback-lines"));
   ui_resize_window();
   ui_io_watcher_init();
   ta_printf(msgbox, "$CYAN$Welcome to ft8goblin, a console ft8 client with support for multiple bands!");
   redraw_screen();

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
   log_close(mainlog);
   mainlog = NULL;
   fini(0);			// remove pidfile, etc
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
   ta_printf(msgbox, "$RED$Halting TX!");
   redraw_screen();
   tb_present();
}
