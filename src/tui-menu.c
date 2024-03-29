#include "config.h"
#include "debuglog.h"
#include "util.h"
#include "tui.h"
#include "ft8goblin_types.h"
#include <errno.h>
#include <termbox2.h>

extern TextArea *msgbox;
menu_history_t menu_history[MAX_MENULEVEL];	// history for ESC (go back) in menus ;)
int	menu_level = 0;		// Which menu level are we in? This is used to keep track of where we are in menu_history

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
   { (char *)NULL, (char *)NULL, 0 }
};

menu_t menu_bands = { "Band Settings", "Configure RX and TX bands here", menu_bands_items };

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
      ta_printf(msgbox, "$RED$You cannot go deeper than %d menu_level. sorry!", MAX_MENULEVEL);
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
   ta_printf(msgbox, "$YELLOW$Menu level %d closed!", menu_level);
   tb_present();

   // remove one item from the end of menu_history...
   menu_history_pop();	
   return 0;
}

menu_window_t *menu_window_render(menu_t *menu, int menu_entries) {
    menu_window_t *wp = NULL;

    if ((wp = malloc(sizeof(menu_window_t))) == NULL) {
       fprintf(stderr, "menu_window_render: out of memory!");
       exit(250);
    }

    wp->name = menu->menu_name;
    wp->title = menu->menu_title;
    wp->x = 0;
    wp->y = 0;

    return wp;
}

int menu_item_render(menu_window_t *mp, menu_t *menu, int menu_item) {
    return 0;
}

// Display a menu, optionally defaulting the cursor to a specific item (for history)
int menu_show(menu_t *menu, int item) {
   if (menu_level >= MAX_MENULEVEL) {
      ta_printf(msgbox, "$RED$You have reached the maximum menu depth allowed (%d), please use ESC to go back some!", MAX_MENULEVEL);
      return -1;
   }

   tb_clear();
   redraw_screen();

   // Add the menu to the menu history 
   menu_history_push(menu, 0);

   ta_printf(msgbox, "$RED$Show menu %s <menu_level:%d>!", menu->menu_name, menu_level);

   menu_item_t **ip = menu->menu_items;
   if (ip == NULL) {
      ta_printf(msgbox, "$RED$Invalid menu data, items pointer is NULL");
      return -1;
   }

   size_t mi_entries = (sizeof(ip) / sizeof(menu[0]));

   // render the outer dialog, with space for mi_entries or scrollbars
   menu_window_t *mp = menu_window_render(menu, mi_entries);

   // render each menu item into the menu
   for (size_t i = 0; i < mi_entries; i++) {
      menu_window_t *newwin = menu_item_render(mp, menu, i);
      // XXX: finish this
      // stop memory leak
      free(newwin);
   }

   // XXX: finish this
   free(mp);

   tb_present();
   return 0;
}
