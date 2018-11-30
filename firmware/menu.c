#include <stddef.h>
#include <string.h>
#include "exception.h"

#include "menu.h"
#include "utils.h"

/* start using a menu, set current item to first item in top level */
void menu_initialise(struct menu *menu) {
    menu->submenu = NULL;
    menu->current_entry = 0;
}

/* empty a menu */
void menu_clear(struct menu *menu) {
    menu_initialise(menu);
    menu->length = 0;
}

/* add an info entry to a menu */
void menu_append_info(struct menu *menu, const char *text){
    struct menu_entry *entry;
    if (menu->length >= menu->max_length) {
        Throw(ERROR_MENU_FULL);
    }
    entry = menu->entries + menu->length;
    SAFE_STRING_COPY(entry->text, text, MENU_TEXT_LENGTH-1);
    entry->type = Info;
    entry->action = NULL;
    entry->argument = 0;
    menu->length++;
}

/* add a submenu entry to a menu */
void menu_append_submenu(struct menu *menu, const char *text, struct menu *submenu){
    struct menu_entry *entry;
    if (menu->length >= menu->max_length) {
        Throw(ERROR_MENU_FULL);
    }
    entry = menu->entries + menu->length;
    SAFE_STRING_COPY(entry->text, text, MENU_TEXT_LENGTH-1);
    entry->type = SubMenu;
    entry->submenu = submenu;
    entry->argument = 0;
    menu->length++;
}

/* add an action entry to a menu */
void menu_append_action(struct menu *menu, const char *text, menu_callback action, int argument){
    struct menu_entry *entry;
    if (menu->length >= menu->max_length) {
        Throw(ERROR_MENU_FULL);
    }
    entry = menu->entries + menu->length;
    SAFE_STRING_COPY(entry->text, text, MENU_TEXT_LENGTH-1);
    entry->type = Action;
    entry->action = action;
    entry->argument = argument;
    menu->length++;
}

/* add a back entry to a menu */
void menu_append_back(struct menu *menu, const char *text){
    struct menu_entry *entry;
    if (menu->length >= menu->max_length) {
        Throw(ERROR_MENU_FULL);
    }
    entry = menu->entries + menu->length;
    SAFE_STRING_COPY(entry->text, text, MENU_TEXT_LENGTH-1);
    entry->type = Back;
    entry->action = NULL;
    entry->argument = 0;
    menu->length++;
}



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


/* undertake the action defined by the menu (go to sub-menu, go back up a level or execute function */
enum action menu_action(struct menu *menu) {
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
        return Action;
        break;
    case SubMenu:
        menu_initialise(entry->submenu);
        menu->submenu = entry->submenu;
        return SubMenu;
        break;
    case Back:
        if (parent) {
            parent->submenu = NULL;
        }
        return Back;
        break;
    case Info:
    default:
        return Info;
        break;
    }
}


