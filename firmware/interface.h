#ifndef INTERACE_H
#define INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "menu.h"

enum INPUT {
    NONE = 0,
    FLIP_DOWN,
    FLIP_UP,
    FLIP_LEFT,
    FLIP_RIGHT,
    SINGLE_CLICK,
    LONG_CLICK,
    DOUBLE_CLICK
};




extern struct menu main_menu;

enum INPUT get_input();
extern volatile enum INPUT last_click;
void show_menu(struct menu *menu);
#endif
