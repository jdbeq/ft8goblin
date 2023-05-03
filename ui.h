#if	!defined(_ui_h)
#define	_ui_h
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include "ui-menu.h"
#include "ui-input.h"
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif
   typedef struct TextArea {
      int	scrollback_lines;	// this is set in main below...
      int	selected_line;
      rb_buffer_t* scrollback;		// the ring buffer for scrollback
      int 	top,			// top of scrollable TextArea
                bottom;			// bottom of scrollable TextArea
      int	x_offset;		// offset from left edge
   } TextArea;

   // Print *anywhere* on the screen (dont try to print on the TextAreas ;)
   extern void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg);
   extern void printf_tb(int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...);

   // Handling of the TextArea
   extern void ta_redraw(TextArea *ta);
   extern int ta_append(TextArea *ta, const char *buf);
   TextArea *ta_init(int scrollback_lines);
   extern void ta_printf(TextArea *ta, const char *fmt, ...);

   // ui functions
   extern void ui_resize_window(void);
   extern void ui_shutdown(void);
   extern void ui_init(void);

   /////
   extern int scrollback_lines;	// this is set in main below...
   extern int active_band;		// Which band are we TXing on?
   extern int active_pane;		// active pane (0: TextArea, 1: TX input)

   extern int height, width;
   extern int line_status;		// status line
   extern int line_input;		// input field

   // stuff pulled once from config...
   extern const char *mycall;		// cfg:ui/mycall
   extern const char *gridsquare;	// cfg:ui/gridsquare

   // These need to move elsewhere...
   extern void halt_tx_now(void);
   extern int view_config(void);
   extern TextArea *msgbox;
   extern int dying;			// Are we shutting down?
   extern int tx_enabled;		// Master toggle to TX mode.
#ifdef __cplusplus
};
#endif
#endif	// !defined(_ui_h)
