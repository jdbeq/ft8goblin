#if	!defined(_ui_h)
#define	_ui_h

#ifdef __cplusplus
extern "C" {
#endif
   ///////////////////
   // forward declare this, so we can use it in menu_t and menu_history_t below
   typedef struct menu_item menu_item_t;

   typedef struct menu_s {
     char	menu_name[MAX_MENUNAME];
     char	menu_title[MAX_MENUTITLE];
     menu_item_t *menu_items[MAX_MENUITEMS];
   } menu_t;

   struct menu_item {
     char	item_name[MAX_MENUNAME];
     char	item_title[MAX_MENUTITLE];
     int	(*callback)();		// Callback to fire when clicked
     /////////////////////////////////////////////////
     // Only one of these can be selected at a time //
     /////////////////////////////////////////////////
     menu_t *item_menu;		// menu target (menu pointer)
     int	  item_menu_item;	// menu item (menu pointer)
   };

   typedef struct menu_history {
      menu_t *menu;
      int	   item;		// menu.menu_items[x]
   } menu_history_t;

   extern menu_history_t menu_history[MAX_MENULEVEL];	// history for ESC (go back) in menus ;)
   extern int menu_history_clear(void);
   extern void menu_history_push(menu_t *menu, int item);
   extern void menu_history_pop(void);
   extern int menu_close(void);
   extern int menu_show(menu_t *menu);
   extern void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg);
   extern void printf_tb(int x, int y, uint16_t fg, uint16_t bg, const char *fmt, ...);
   extern void ta_redraw(void);
   extern int ta_append(const char *buf);

   /////
   // These need to move elsewhere...
   extern void halt_tx_now(void);
   extern int view_config(void);
#ifdef __cplusplus
};
#endif
#endif	// !defined(_ui_h)
