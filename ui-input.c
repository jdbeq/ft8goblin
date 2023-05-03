//
// Keyboard/mouse input handler
//   
#include <termbox2.h>
#include "config.h"
#include "ui.h"

void process_input(struct tb_event *evt) {
   if (evt == NULL) {
      // XXX: log the error!
      ta_printf("$RED$%s called with ev == NULL wtf?!", __FUNCTION__);
      tb_present();
      return;
   }

   if (evt->type == TB_EVENT_KEY) {
      // Is the key a valid command?
      if (evt->key == TB_KEY_ESC) {
         if (menu_level > 0) {
            menu_close();
         } else {
           menu_level = 0; // reset it to zero
           ta_printf("$YELLOW$The menu is already closed!");
         }
      } else if (evt->key == TB_KEY_TAB) {
        if (menu_level == 0) {		// only apply in main screen
           if (active_pane == 0)
              active_pane = 1;
           else
              active_pane = 0;
        }
      } else if (evt->key == TB_KEY_ARROW_LEFT) { 			// left cursor
         ta_printf("$YELLOW$<");
      } else if (evt->key == TB_KEY_ARROW_RIGHT) {			// right cursor
         ta_printf("$YELLOW$>");
      } else if (evt->key == TB_KEY_ARROW_UP) {			// up cursor
         ta_printf("$YELLOW$^");
      } else if (evt->key == TB_KEY_ARROW_DOWN) {			// down cursor
         ta_printf("$YELLOW$V");
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
            redraw_screen();
            ta_printf("$RED$TX %sabled globally!", (tx_enabled ? "en" : "dis"));
         } else {
            // always disable if in a submenu, only allow activating TX from home screen
            tx_enabled = 0;
            ta_printf("$RED$TX %sabled globally!", (tx_enabled ? "en" : "dis"));
         }
      } else if (evt->key == TB_KEY_CTRL_X || evt->key == TB_KEY_CTRL_Q) {	// is it ^X or ^Q? If so exit
         ta_printf("$RED$Goodbye! Hope you had a nice visit!");
         ui_shutdown();
         return;
      } else {      					// Nope - display the event data for debugging
         ta_printf("$RED$unknown event: type=%d key=%d ch=%c", evt->type, evt->key, evt->ch);
      }
   } else if (evt->type == TB_EVENT_RESIZE) {
      // change the stored dimensions/layout variables above
      ui_resize_window();

      // clear the screen buffer
      tb_clear();

      // redraw the various sections of the screen
      redraw_screen();
   } else if (evt->type == TB_EVENT_MOUSE) {
      // handle mouse interactions
   }
   tb_present();
}
