#ifndef INTERACE_H
#define INTERFACE_H
#include <stdint.h>
#include <stdbool.h>

#define TRIS_BUTTON TRISBbits.TRISB7
#define PORT_BUTTON PORTBbits.RB7
enum ACTION {
    NONE = 0,
    FLIP_DOWN,
    FLIP_UP,
    FLIP_LEFT,
    FLIP_RIGHT,
    SINGLE_CLICK,
    LONG_CLICK,
    DOUBLE_CLICK
};

#define FIRST_MENU_ITEM 1
enum ACTION get_action();
extern volatile enum ACTION last_click;
bool show_menu(int16_t index, bool first_time);
#endif
