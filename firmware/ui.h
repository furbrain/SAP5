/* 
 * File:   ui.h
 * Author: phil
 *
 * Created on 12 April 2020, 21:25
 */

#ifndef UI_H
#define	UI_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include "exception.h"
#define UI_SELECT_MAX_POS 3
    
struct UI_MULTI_SELECT;

typedef void (*ui_multi_cb_t)(struct UI_MULTI_SELECT*);

struct UI_NUMBER_DEF {
    int max;
    int min;
    int current;
    int len;
    int pos;
    ui_multi_cb_t callback;
};

struct UI_MULTI_SELECT {
    char text[12];
    int offset;
    struct UI_NUMBER_DEF numbers[UI_SELECT_MAX_POS];
    int _offsets[12];
};

void ui_multi_select(struct UI_MULTI_SELECT *sel);
bool ui_yes_no(const char *text);
bool get_single_click(void);
void get_single_click_or_throw(char *reason, enum EXCEPTION_CODES code);
#ifdef TEST
void draw_markers(int column);
void erase_markers(int column);
void erase_character(int column);

int get_digit(int num, int pos);
int set_digit(int num, int pos, int digit);
bool compare_nums(int a, int b, int pos);

extern const uint8_t up_marker[15];
extern const uint8_t down_marker[15];
extern const uint8_t empty_marker[15];
#endif
#endif	/* UI_H */

