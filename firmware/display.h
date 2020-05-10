#ifndef DISPLAY_H
#define DISPLAY_H

#include "app_type.h"
#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define NUM_PAGES 8

typedef uint8_t display_buf_t[NUM_PAGES][128];

void display_init();

void display_close();

void display_on();

void display_off();

void display_clear(bool immediate);

void display_set_brightness(uint8_t brightness);

void render_data_to_screen(uint8_t page, uint8_t column, const uint8_t* data, uint8_t length);


#ifndef BOOTLOADER
#include "font.h"
void display_load_buffer(uint8_t page, uint8_t column, const uint8_t* data, uint8_t length);

void display_write_text(int page, int column, const char* text, const struct FONT *font, bool right_justify);

void display_write_multiline(int page, const char* text, bool immediate);

void display_rle_image(const char image[]);

int render_text_to_page(uint8_t *buffer, int page, int column, const char *text, const struct FONT *font);

void display_swipe_pages(int start_page, int page_count, bool left);

void display_scroll_buffer(bool up);

void display_flip(bool invert);

void display_set_day(bool day);

void display_show_buffer(void);
void display_setbuffer_xy(int x, int y);
void display_clearbuffer_xy(int x, int y);
void display_draw_line(int x0, int y0, int x1, int y1);
extern display_buf_t display_screen;
extern display_buf_t display_buffer;
#endif
#endif
