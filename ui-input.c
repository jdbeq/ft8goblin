//
// Keyboard/mouse input handler
//   
#include <termbox2.h>
#include "config.h"
#include "ui.h"
#include "util.h"
#include "logger.h"
#include <ev.h>

static ev_io termbox_watcher, termbox_resize_watcher;
extern TextArea *msgbox;

void process_input(struct tb_event *evt) {
   if (evt == NULL) {
      log_send(mainlog, LOG_CRIT, "process_input: called with ev == NULL. This shouldn't happen!");
      return;
   }

   if (evt->type == TB_EVENT_KEY) {
      // Is the key a valid command?
      if (evt->key == TB_KEY_ESC) {
         if (menu_level > 0) {
            menu_close();
         } else {
           menu_level = 0; // reset it to zero
         }
      } else if (evt->key == TB_KEY_TAB) {
        if (menu_level == 0) {		// only apply in main screen
           if (active_pane == 0)
              active_pane = 1;
           else
              active_pane = 0;
        }
      } else if (evt->key == TB_KEY_ARROW_LEFT) { 			// left cursor
      } else if (evt->key == TB_KEY_ARROW_RIGHT) {			// right cursor
      } else if (evt->key == TB_KEY_ARROW_UP) {			// up cursor
      } else if (evt->key == TB_KEY_ARROW_DOWN) {			// down cursor
      } else if (evt->key == TB_KEY_CTRL_B) {			// ^B
         if (menu_level == 0) {			// only if we're at main TUI screen (not in a menu)
            menu_show(&menu_bands, 0);
         }
      } else if (evt->key == TB_KEY_CTRL_H) {			// ^H
         tx_enabled = 0;
         halt_tx_now();
      } else if (evt->key == TB_KEY_CTRL_M) { 			// Is it ^M?
         if (menu_level == 0) {
            menu_history_clear();
            menu_show(&menu_main, 0);
         } else {
            // pass ^M through
         }
      } else if (evt->key == TB_KEY_CTRL_T) {		// ^T
         if (menu_level == 0) {
            toggle(&tx_enabled);
            redraw_screen();
            ta_printf(msgbox, "$RED$TX %sabled globally!", (tx_enabled ? "en" : "dis"));
         } else {
            // always disable if in a submenu, only allow activating TX from home screen
            tx_enabled = 0;
            ta_printf(msgbox, "$RED$TX %sabled globally!", (tx_enabled ? "en" : "dis"));
         }
      } else if (evt->key == TB_KEY_CTRL_X || evt->key == TB_KEY_CTRL_Q) {	// is it ^X or ^Q? If so exit
         ta_printf(msgbox, "$RED$Goodbye! Hope you had a nice visit!");
         dying = 1;
         ui_shutdown();
         exit(0);
         return;
      } else {      					// Nope - display the event data for debugging
         ta_printf(msgbox, "$RED$unknown event: type=%d key=%d ch=%c", evt->type, evt->key, evt->ch);
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

//////////////////////////////////////////
// Handle termbox tty and resize events //
//////////////////////////////////////////
static void termbox_cb(EV_P_ ev_io *w, int revents) {
   struct tb_event evt;		// termbox io events
   tb_poll_event(&evt);
   process_input(&evt);
}

int ui_io_watcher_init(void) {
   struct ev_loop *loop = EV_DEFAULT;
   int rv = 0;
   int fd_tb_tty = -1, fd_tb_resize = -1;

   ///////////////////////////////////////////
   // Setup libev to handle termbox2 events //
   ///////////////////////////////////////////
   tb_get_fds(&fd_tb_tty, &fd_tb_resize);

   // stdio occupy 0-2 (stdin, stdout, stderr)
   if (fd_tb_tty >= 2 && fd_tb_resize >= 2) {
      // add to libev set
   } else {
      ta_printf(msgbox, "$RED$tb_get_fds returned nonsense (%d, %d) can't continue!", fd_tb_tty, fd_tb_resize);
      tb_present();
      exit(200);
   }

   // XXX: error checking!
   ev_io_init(&termbox_watcher, termbox_cb, fd_tb_tty, EV_READ);
   ev_io_init(&termbox_resize_watcher, termbox_cb, fd_tb_resize, EV_READ);
   ev_io_start(loop, &termbox_watcher);
   ev_io_start(loop, &termbox_resize_watcher);

   return rv;
}
