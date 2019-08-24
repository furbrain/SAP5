#ifndef INTERACE_H
#define INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "menu.h"




extern struct menu main_menu;

void show_menu(struct menu *menu);
void show_status();
#endif
