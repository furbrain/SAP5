#include <stdbool.h>
#include "selector.h"
#include "display.h"
#include "input.h"
#include "utils.h"

const uint8_t up_marker[15] = {0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
                              0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
const uint8_t down_marker[15] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
                               0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};
const uint8_t empty_marker[15] = {0};


TESTABLE_STATIC
void draw_markers(int column) {
    render_data_to_page(1, column, up_marker,15);
    render_data_to_page(6, column, down_marker,15);
}

TESTABLE_STATIC
void erase_markers(int column) {
    render_data_to_page(1, column, empty_marker,15);
    render_data_to_page(6, column, empty_marker,15);
}

TESTABLE_STATIC
void erase_character(int column) {
    render_data_to_page(2, column, empty_marker,14);
    render_data_to_page(3, column, empty_marker,14);
    render_data_to_page(4, column, empty_marker,14);
    render_data_to_page(5, column, empty_marker,14);
}

/* ensure selected current is valid*/
TESTABLE_STATIC
void correct_selector(struct SELECTOR_CHOICES *c) {
    if (c->current >= c->end) c->current = c->start;
    if (c->current < c->start) c->current = c->end-1;
}

void selector_write_char(char text, int column) {
    char one_char[] = "X";
    one_char[0] = text;
    display_write_text(2, column, one_char, &large_font, false, true);
}



/* display a selector at given page and column
 * the char will be displayed at page 2, column in large font
 * up and down markers will be displayed above it *
 * return the character selected */
char selector_choose(struct SELECTOR_CHOICES *c, int column) {
    draw_markers(column);
    correct_selector(c);
    selector_write_char(c->text[c->current], column);
            
    while(true) {
        switch(get_input()) {
            case SINGLE_CLICK:
                erase_markers(column);
                return c->text[c->current];
                break;
            case FLIP_UP:
                ++c->current;
                correct_selector(c);
                erase_character(column);
                selector_write_char(c->text[c->current], column);
                delay_ms_safe(500);
                break;
            case FLIP_DOWN:
                --c->current;
                correct_selector(c);
                erase_character(column);
                selector_write_char(c->text[c->current], column);
                delay_ms_safe(500);
                break;
            default:
                break;
        }
        delay_ms_safe(10);
    }
}
