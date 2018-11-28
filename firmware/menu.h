#ifndef _MENU_H
#define _MENU_H
#include <stdint.h>

enum action {
    Action,
    Back,
    Info,
    SubMenu
};

#define DECLARE_MENU(name, entries...) \
    struct menu_entry name##_entries[] = entries;\
    struct menu name = {0, \
                        sizeof(name##_entries) / sizeof(name##_entries[0]), \
                        NULL, \
                        name##_entries}
struct menu;
    
struct menu_entry{
    char* text;
    enum action type; 
    union {
        void (*action) (int); /*action to perform when selected */
        struct menu *submenu; /*submenu to jump to */
    };
    int32_t argument; /* argument to pass to action (if needed) */
};

struct menu{
    int16_t current_entry;
    int16_t length;
    struct menu *submenu;
    struct menu_entry *entries;
};

/* move the menu to the next item, wrapping if necessary */
void menu_next(struct menu *menu);

/* move the menu to the previous item, wrapping if necessary */
void menu_prev(struct menu *menu);

/* get the menu text, do not alter the returned string, only valid as long as the underlying menu */
char* menu_get_text(struct menu *menu);

/* start using a menu, set current item to first item in top level */
void menu_initialise(struct menu *menu);

/* undertake the action defined by the menu (go to sub-menu, go back up a level or execute function */
enum action menu_action(struct menu *menu);


#endif // _MENU_H
