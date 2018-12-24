#ifndef _MENU_H
#define _MENU_H
#include <stdint.h>

#define MENU_TEXT_LENGTH 15

enum action {
    Action,
    Back,
    Info,
    SubMenu,
    Exit
};

typedef void (*menu_callback)(int);

#define DECLARE_MENU(name, entries...) \
    struct menu_entry name##_entries[] = entries;\
    struct menu name = {0, \
                        sizeof(name##_entries) / sizeof(name##_entries[0]), \
                        sizeof(name##_entries) / sizeof(name##_entries[0]), \
                        NULL, \
                        name##_entries}

#define DECLARE_EMPTY_MENU(name, count) \
    struct menu_entry name##_entries[count] = {0};\
    struct menu name = {0, \
                        0, \
                        count, \
                        NULL, \
                        name##_entries}
struct menu;
    
struct menu_entry{
    char text[MENU_TEXT_LENGTH];
    enum action type; 
    union {
        menu_callback action; /*action to perform when selected */
        struct menu *submenu; /*submenu to jump to */
    };
    int32_t argument; /* argument to pass to action (if needed) */
};

struct menu{
    int16_t current_entry;
    int16_t length;
    int16_t max_length;
    struct menu *submenu;
    struct menu_entry *entries;
};

/* empty a menu */
void menu_clear(struct menu *menu);

/* add an info entry to a menu */
void menu_append_info(struct menu *menu, const char *text);

/* add a submenu entry to a menu */
void menu_append_submenu(struct menu *menu, const char *text, struct menu *submenu);

/* add an action entry to a menu */
void menu_append_action(struct menu *menu, const char *text, void (*action) (int), int argument);

/* add a back entry to a menu */
void menu_append_back(struct menu *menu, const char *text);

/* add an exit entry to a menu */
void menu_append_exit(struct menu *menu, const char *text);

/* move the menu to the next item, wrapping if necessary */
void menu_next(struct menu *menu);

/* move the menu to the previous item, wrapping if necessary */
void menu_prev(struct menu *menu);

/* get the menu text, do not alter the returned string, only valid as long as the underlying menu */
const char* menu_get_text(struct menu *menu);

/* start using a menu, set current item to first item in top level */
void menu_initialise(struct menu *menu);

/* undertake the action defined by the menu (go to sub-menu, go back up a level or execute function */
enum action menu_action(struct menu *menu);


#endif // _MENU_H
