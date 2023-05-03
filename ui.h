#if	!defined(_ui_h)
#define	_ui_h
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include "menu.h"

#ifdef __cplusplus
extern "C" {
#endif
   // Print *anywhere* on the screen (dont try to print on the TextArea)
   extern void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg);
   extern void printf_tb(int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...);
   // Handling of the TextArea
   extern void ta_redraw(void);
   extern int ta_append(const char *buf);
   extern void ta_printf(const char *fmt, ...);

   /////
   extern int	scrollback_lines;	// this is set in main below...
   extern int	active_band;		// Which band are we TXing on?
   extern int	active_pane;		// active pane (0: TextArea, 1: TX input)

   extern int height, width;
   extern int line_textarea_top,	// top of scrollable TextArea
       line_textarea_bottom;		// bottom of scrollable TextArea
   extern int line_status;		// status line
   extern int line_input;		// input field
   extern const char *mycall;		// cfg:ui/mycall
   extern const char *gridsquare;	// cfg:ui/gridsquare

   // These need to move elsewhere...
   extern void halt_tx_now(void);
   extern int view_config(void);
   extern int dying;			// Are we shutting down?
   extern int tx_enabled;		// Master toggle to TX mode.
#ifdef __cplusplus
};
#endif
#endif	// !defined(_ui_h)
