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
    char* text;
    int16_t next_menu; /* start of next menu, or one of FUNCTION, BACK, INFO*/
    void (*action) (int); /*action to perform when selected */
    int32_t argument; /* argument to pass to action (if needed) */
};


#define FUNCTION -1
#define BACK -2
#define INFO -3

extern const struct menu_entry main_menu[];

void menu_set_entry(struct menu_entry *menu, int16_t index, char *text, int16_t next_menu, void (*action) (int), int32_t argument);

#define FIRST_MENU_ITEM 1
enum ACTION get_action();
extern volatile enum ACTION last_click;
bool show_menu(const struct menu_entry *menu, int16_t index, bool first_time);
#endif
