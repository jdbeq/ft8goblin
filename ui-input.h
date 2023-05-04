#if	!defined(_ui_input_h)
#define	_ui_input_h
#include <termbox2.h>

#define	MAX_KEY_ID	0xffff

#ifdef __cplusplus
extern "C" {
#endif
   // you need to create an array of these, with the desired key as index
   typedef struct Keymap {
      void (*callback)();
   } Keymap;

   extern void process_input(struct tb_event *evt);
   extern int ui_io_watcher_init(void);
   extern struct ev_loop *loop;
   extern time_t now;
#ifdef __cplusplus
};
#endif

#endif	// !defined(_ui_input_h)
