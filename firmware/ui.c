#include <stdlib.h>
#include <string.h> //FIXME
#include "ui.h"
#include "utils.h"
#include "display.h"
#include "input.h"
#include "font.h"
#include "exception.h"

TESTABLE_STATIC
const uint8_t up_marker[15] = {0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
                              0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};

TESTABLE_STATIC
const uint8_t down_marker[15] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
                               0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
TESTABLE_STATIC
const uint8_t empty_marker[15] = {0};

static
const char digits[]="0123456789";

TESTABLE_STATIC
void draw_markers(int column) {
    display_load_buffer(1, column, up_marker,15);
    display_load_buffer(6, column, down_marker,15);
}

TESTABLE_STATIC
void erase_markers(int column) {
    display_load_buffer(1, column, empty_marker,15);
    display_load_buffer(6, column, empty_marker,15);
}

TESTABLE_STATIC
void erase_character(int column) {
    display_load_buffer(2, column, empty_marker,14);
    display_load_buffer(3, column, empty_marker,14);
    display_load_buffer(4, column, empty_marker,14);
    display_load_buffer(5, column, empty_marker,14);
}

TESTABLE_STATIC
void write_char(char c, int pos) {
    char one_char[] = "X";
    one_char[0] = c;
    display_write_text(2, pos, one_char, &large_font, false);
}

static
int get_pos(int offset, int index) {
    return offset + index *13;
}

TESTABLE_STATIC
int get_digit(int num, int pos) {
    while (pos--) {
        num /= 10;
    }
    return num % 10;
}

TESTABLE_STATIC
int set_digit(int num, int pos, int digit) {
    int power = 1;
    int rhs;
    while (pos--) {
        power *= 10;
    }
    rhs = num % power;
    num -= num % (power*10);
    num += digit * power;
    num += rhs;
    return num;
}

TESTABLE_STATIC
bool compare_nums(int a, int b, int pos) {
    while (pos--) {
        a /= 10;
        b /= 10;
    }
    return (a==b);
}

int
select_char(const char *list, int min_dig, int max_dig, int current, int column) {
    bool update_char = true;
    draw_markers(column);
    while(true) {
        if (update_char) {
            if(current > max_dig) {
                current = min_dig;
            }
            if (current < min_dig) {
                current = max_dig;
            }
            erase_character(column);
            write_char(list[current], column);
            display_show_buffer();
            update_char = false;
            delay_ms_safe(500);            
        }
        switch(get_input()) {
            case SINGLE_CLICK:
                erase_markers(column);
                return current;
                break;
            case FLIP_UP:
                ++current;
                update_char = true;
                break;
            case FLIP_DOWN:
                --current;
                update_char = true;
                break;
            default:
                break;
        }
        delay_ms_safe(10);
    }
}

TESTABLE_STATIC
void process_number(struct UI_NUMBER_DEF *num, int offset) {
    int digit = 0;
    int power;
    
    int min_digit, max_digit, current;
    for (digit = 0; digit < num->len; digit++) {
        min_digit = 0;
        max_digit = 9;
        power = num->len - digit - 1;
        if (compare_nums(num->current, num->min, power + 1)) {
            min_digit = get_digit(num->min, power);
        }
        if (compare_nums(num->current, num->max, power + 1)) {
            max_digit = get_digit(num->max, power);
        }
        current = get_digit(num->current, power);
        current = select_char(digits, min_digit, max_digit, current, get_pos(offset, digit));
        num->current = set_digit(num->current, power, current);
    }
}

static
void calc_offsets(struct UI_MULTI_SELECT *sel) {
    const struct GLYPH_DATA *glyph;
    int i = 0;
    int j;
    int len = strlen(sel->text);
    int num = 0;
    int offset = sel->offset;
    while (i < len) {
        if ((num < UI_SELECT_MAX_POS) && (i >= sel->numbers[num].pos)) {
            for (j=0; j< sel->numbers[num].len; j++) {
                sel->_offsets[i] = offset;
                offset += 14;
                i++;
            }
            num++;
        } else {
            sel->_offsets[i] = offset;
            glyph = font_get_glyph_data(&large_font, sel->text[i]);
            offset += glyph->width;
            offset += large_font.advance;
            i++;
        }
    }
}

static
void select_init(struct UI_MULTI_SELECT *sel) {
    int i;    
    display_clear(false);
    for (i=0; sel->text[i] != '\0'; i++) {
        write_char(sel->text[i], sel->_offsets[i]);
    }
    display_show_buffer();
}

void ui_multi_select(struct UI_MULTI_SELECT *sel) {
    int i;
    calc_offsets(sel);
    select_init(sel);
    for (i=0; i<UI_SELECT_MAX_POS; i++) {
        process_number(&sel->numbers[i], sel->_offsets[sel->numbers[i].pos]);
        if (sel->numbers[i].callback) {
            sel->numbers[i].callback(sel);
        }
    }
}

static
void draw_choice(bool choice) {
    if (choice) {
        display_write_text(0, 128, ">Y", &large_font, true);
        display_write_text(4, 128, "       N", &large_font, true);
    } else {
        display_write_text(0, 128, "       Y", &large_font, true);
        display_write_text(4, 128, ">N", &large_font, true);
    }
}

bool ui_yes_no(const char *text) {
    char tmp_text[40];
    bool choice = true;
    strcpy(tmp_text, text);
    char *linebreak = strchr(tmp_text, '\n');
    display_clear(false);
    if (linebreak) {
        *linebreak = '\0';
        display_write_text(0, 0, tmp_text, &large_font, false);
        display_write_text(4, 0, linebreak+1, &large_font, false);
    } else {
        display_write_text(2, 0, tmp_text, &large_font, false);
    }
    draw_choice(choice);
    display_show_buffer();
    while (true) {
        switch(get_input()) {
            case FLIP_DOWN:
                choice  = !choice;
                //display_clear(false);
                draw_choice(choice);
                display_show_buffer();
                break;
            case SINGLE_CLICK:
                return choice;
            default:
                break;
        }
        delay_ms_safe(10);
    }
}

bool get_single_click(void) {
    while (true) {
        switch(get_input()) {
            case SINGLE_CLICK:
                return true;
                break;
            case SHAKE:
            case LONG_CLICK:
            case DOUBLE_CLICK:
                return false;
                break;
            default:
                break;
        }
        delay_ms_safe(10);        
    }
}

void get_single_click_or_throw(char *reason, enum EXCEPTION_CODES code) {
    if (!get_single_click()) {
        THROW_WITH_REASON(reason, code);
    }
}
