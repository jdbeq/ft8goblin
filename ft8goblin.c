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
#include "logger.h"

char *progname = "ft8goblin";

/////////////////////////////////////////
int	dying = 0;		// Are we shutting down?
int	tx_enabled = 0;		// Master toggle to TX mode.

static void exit_fix_config(void) {
   printf("Please edit your config.json and try again!\n");
   exit(255);
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

   mainlog = log_open(cfg_get_str(cfg, "logging/ui-logpath"));

   ui_init();
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
