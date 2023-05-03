#if	!defined(_ui_input_h)
#define	_ui_input_h

#ifdef __cplusplus
extern "C" {
#endif
   extern void process_input(struct tb_event *evt);
   extern int ui_io_watcher_init(void);
   extern struct ev_loop *loop;
#ifdef __cplusplus
};
#endif

#endif	// !defined(_ui_input_h)
