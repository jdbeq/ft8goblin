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
#include <ev.h>
#include <evutil.h>
#include "config.h"
#include "termbox2/termbox2.h"
#include "subproc.h"
#include "ui.h"
#include "util.h"

// forward prototypes
static void fini(void);
static void process_input(struct tb_event *evt);

/////////////////////////////////////////
menu_history_t menu_history[MAX_MENULEVEL];	// history for ESC (go back) in menus ;)
int	menu_level = 0;		// Which menu level are we in? This is used to keep track of where we are in menu_history
int	dying = 0;		// Are we shutting down?
int	tx_enabled = 0;		// Master toggle to TX mode.

// X and Y coordinates of the display
int	x = 0, y = 0;		// display coordinates

// main menu items
menu_item_t menu_main_items[] = {
   { "View config", "View the active json configuration", view_config },
   { (char *)NULL, (char *)NULL, (int *)NULL }
};

// main menu metadata
menu_t menu_main = { "Main Menu", "This is the main menu", menu_main_items };

// XXX: This needs generated from the config, but some examples to get started...
menu_item_t menu_bands_items[] = {
   { "160 Meters FT8", "Toggle RX/TX on ft8-160m", NULL },
   { "80 Meters FT8", "Toggle RX/TX on ft8-80m", NULL },
   { "60 Meters FT8", "Toggle RX/TX on ft8-60m", NULL },
   { "40 Meters FT8", "Toggle RX/TX on ft8-40m", NULL },
   { "30 Meters FT8", "Toggle RX/TX on ft8-30m", NULL },
   { "20 Meters FT8", "Toggle RX/TX on ft8-20m", NULL },
   { "17 Meters FT8", "Toggle RX/TX on ft8-17m", NULL },
   { "15 Meters FT8", "Toggle RX/TX on ft8-15m", NULL },
   { "12 Meters FT8", "Toggle RX/TX on ft8-12m", NULL },
   { "10 Meters FT8", "Toggle RX/TX on ft8-10m", NULL },
   { "6 Meters FT8", "Toggle RX/TX on ft8-6m", NULL },
   { NULL, NULL, NULL }
};

menu_t menu_bands = { "Band Settings", "Configure RX and TX bands here", menu_bands_items };

static void exit_fix_config(void) {
   printf("Please edit your config.json and try again!\n");
   exit(255);
}

static void print_help(void) {
   tb_printf(0, 0, TB_BLUE|TB_BOLD, 0, "Keys: ESC Exit Menu, ^Q / ^X Exit, ^B Bands Menu, ^M Menu, ^T Toggle TX, ^H Halt TX immediately");
   tb_present();
}

int menu_history_clear(void) {
   // clear out al the menu_history pointers
   for (int i = 0; i < MAX_MENULEVEL; i++) {
       menu_history[i].menu = NULL;
       menu_history[i].item = -1;
   }

   // and reset menu level to 0...
   menu_level = 0;
   return 0;
}

void menu_history_push(menu_t *menu, int item) {
   if (menu_level >= MAX_MENULEVEL) {
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "You cannot go deeper than %d menu_level. sorry!", MAX_MENULEVEL);
      tb_present();
      return;
   }
   menu_history[menu_level].menu = menu;
   menu_history[menu_level].item = item;
   menu_level++;
}

void menu_history_pop(void) {
   menu_history_t *mp = NULL;

   if (menu_level <= 0) {
      return;
   }

   mp = &menu_history[menu_level - 1];
   mp->menu = NULL;
   mp->item = -1;
   menu_level--;
}

int menu_close(void) {
   tb_printf(0, y++, TB_RED, 0, "Menu level %d closed!", menu_level);
   tb_present();

   // remove one item from the end of menu_history...
   menu_history_pop();	
   return 0;
}

int menu_show(menu_t *menu) {
   if (menu_level >= MAX_MENULEVEL) {
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "You have reached the maximum menu depth allowed (%d), please use ESC to go back some!", MAX_MENULEVEL);
      return -1;
   }

   tb_clear();
   print_help();
   y = 1;

   // Add the menu to the menu history 
   menu_history_push(menu, 0);

   tb_printf(0, y++, TB_RED|TB_BOLD, 0, "Show menu %s <menu_level:%d>!", menu->menu_name, menu_level);
   tb_present();
   return 0;
}

static void termbox_cb(EV_P_ ev_timer *w, int revents) {
   struct tb_event evt;		// termbox io events
   tb_poll_event(&evt);
   process_input(&evt);
}

//
// Keyboard/mouse input handler
//   
static void process_input(struct tb_event *evt) {
   if (evt == NULL) {
      // XXX: log the error!
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "%s called with ev == NULL wtf?!", __FUNCTION__);
      tb_present();
      return;
   }

   // Is the key a valid command?
   if (evt->key == TB_KEY_ESC) {
      if (menu_level > 0) {
         menu_close();
      } else {
        menu_level = 0; // reset it to zero
        tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "The menu is already closed!");
      }
   } else if (evt->key == TB_KEY_ARROW_LEFT) { 			// left cursor
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "<");
   } else if (evt->key == TB_KEY_ARROW_RIGHT) {			// right cursor
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, ">");
   } else if (evt->key == TB_KEY_ARROW_UP) {			// up cursor
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "^");
   } else if (evt->key == TB_KEY_ARROW_DOWN) {			// down cursor
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "V");
   } else if (evt->key == TB_KEY_CTRL_B) {			// ^B
      if (menu_level == 0) {			// only if we're at main TUI screen (not in a menu)
         menu_show(&menu_bands);
      }
   } else if (evt->key == TB_KEY_CTRL_H) {			// ^H
      tx_enabled = 0;
      halt_tx_now();
   } else if (evt->key == TB_KEY_CTRL_M) { 			// Is it ^M?
      if (menu_level == 0) {
         menu_history_clear();
         menu_show(&menu_main);
      } else {
         // pass ^M through
      }
   } else if (evt->key == TB_KEY_CTRL_T) {		// ^T
      if (menu_level == 0) {
         toggle(&tx_enabled);
         tb_printf(0, y++, TB_RED|TB_BOLD, 0, "TX %sabled globally!", (tx_enabled ? "en" : "dis"));
      } else {
         // always disable if in a submenu, only allow activating TX from home screen
         tx_enabled = 0;
         tb_printf(0, y++, TB_RED|TB_BOLD, 0, "TX %sabled globally!", (tx_enabled ? "en" : "dis"));
      }
   } else if (evt->key == TB_KEY_CTRL_X || evt->key == TB_KEY_CTRL_Q) {	// is it ^X or ^Q? If so exit
      tb_printf(0, y++, TB_RED, 0, "Goodbye! Hope you had a nice visit!");
      fini();
      return;
   } else {      					// Nope - display the event data for debugging
      tb_printf(0, y++, TB_YELLOW|TB_BOLD, 0, "unknown event: type=%d key=%d ch=%c", evt->type, evt->key, evt->ch);
   }
   tb_present();
}

static void fini(void) {
   // Tear down to exit
   tb_clear();
   tb_printf(0, 0, TB_RED|TB_BOLD, 0, "ft8goblin exiting, please wait for subpprocesses to halt...");
   tb_present();
   dying = 1;

   // libev_shutdown();
   subproc_shutdown();

   // shut down termbox
   tb_shutdown();

   printf("ft8goblin exited cleanly!\n");
   exit(0);
}

int main(int argc, char **argv) {
   int fd_tb_tty = -1, fd_tb_resize = -1;
   ev_io termbox_watcher;
   struct ev_loop *loop = EV_DEFAULT;

   // print this even though noone will see it, except in case of error exit ;)
   printf("ft8goblin: A console based ft8 client with support for multiband operation\n\n");

   // This can't work without a valid configuration...
   if (!(cfg = load_config()))
      exit_fix_config();

   ///////////////////////////
   // Perform startup tasks //
   ///////////////////////////

   // Initialize termbox
   tb_init();
   y = 1;
   tb_printf(0, y++, TB_GREEN|TB_BOLD, 0, "Welcome to ft8goblin, a console ft8 client with support for multiple bands!");

   int height = tb_height(), width = tb_width();;

   if (width < 80 || height < 20) {
      tb_clear();
      tb_present();
      tb_shutdown();
      fprintf(stderr, "[display] Your terminal has a size of %dx%d, this is too small! I cannot continue...\n", width, height);
      dying = 1;
      exit(200);
   } else {
      tb_printf(0, y++, TB_GREEN, 0, "[display] Resolution %dx%d is acceptable!", width, height);
   }
   print_help();
   y++;

   ///////////////////////////////////////////
   // Setup libev to handle termbox2 events //
   ///////////////////////////////////////////
   tb_get_fds(&fd_tb_tty, &fd_tb_resize);

   // stdio occupy 0-2 (stdin, stdout, stderr)
   if (fd_tb_tty >= 2 && fd_tb_resize >= 2) {
      // add to libev set
   } else {
      tb_printf(0, y++, TB_RED|TB_BOLD, 0, "tb_get_fds returned nonsense (%d, %d) can't continue!", fd_tb_tty, fd_tb_resize);
      tb_present();
      exit(200);
   }
   ev_io_init (&termbox_watcher, termbox_cb, fd_tb_tty, EV_READ);
   ev_io_start (loop, &termbox_watcher);

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

   fini();
   return 0;
}

///////////
// Menus //
///////////
int view_config(void) {
   //
   return 0;
}

void halt_tx_now(void) {
   tb_printf(0, y++, TB_RED|TB_BOLD, 0, "Halting TX!");
   tb_present();
}
