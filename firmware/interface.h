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
extern volatile enum INPUT last_click;

void interface_init();
enum INPUT get_input();
void show_menu(struct menu *menu);
void show_status();
void timeout_reset();
#endif
