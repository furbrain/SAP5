#include <stddef.h>

#include "menu.h"

/* move the menu to the next item, wrapping if necessary */
void menu_prev(struct menu *menu) {
    while (menu->submenu) 
        menu = menu->submenu;
    menu->current_entry--;
    menu->current_entry += menu->length;
    menu->current_entry %= menu->length;
}

/* move the menu to the previous item, wrapping if necessary */
void menu_next(struct menu *menu) {
    while (menu->submenu) 
        menu = menu->submenu;
    menu->current_entry++;
    menu->current_entry %= menu->length;
}

/* get the menu text, do not alter the returned string, only valid as long as the underlying menu */
char* menu_get_text(struct menu *menu){
    while (menu->submenu) 
        menu = menu->submenu;
    return menu->entries[menu->current_entry].text;
}

/* start using a menu, set current item to first item in top level */
void menu_initialise(struct menu *menu) {
    menu->submenu = NULL;
    menu->current_entry = 0;
}


/* undertake the action defined by the menu (go to sub-menu, go back up a level or execute function */
void menu_action(struct menu *menu) {
    struct menu_entry *entry;
    struct menu *parent = NULL;
    while (menu->submenu) { 
        parent = menu;
        menu = menu->submenu;
    }
    entry = menu->entries + menu->current_entry;
    switch (entry->type) {
    case Action:
        entry->action(entry->argument);
        break;
    case SubMenu:
        menu_initialise(entry->submenu);
        menu->submenu = entry->submenu;
        break;
    case Back:
        if (parent) {
            parent->submenu = NULL;
        }
        break;
    case Info:
        break;
    }
}


