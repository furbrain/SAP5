#ifndef INTERACE_H
#define INTERFACE_H
#include <stdint.h>
#include <stdbool.h>

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

struct menu_entry {
    int16_t index;
    const char* text;
    int16_t next_menu; /* start of next menu (or this menu if text NULL */
    void (*action) (int); /*action to perform when selected */
    int32_t argument; /* argument to pass to action (if needed) */
};

extern const struct menu_entry main_menu[];

#define FIRST_MENU_ITEM 1
enum ACTION get_action();
extern volatile enum ACTION last_click;
bool show_menu(const struct menu_entry *menu, int16_t index, bool first_time);
#endif
