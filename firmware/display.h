#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

extern bool display_inverted;
void display_init();

void display_close();

void display_on(bool enable);

void display_clear_page(uint8_t page);

void display_clear_screen();

void display_set_brightness(uint8_t brightness);

void render_data_to_page(uint8_t page, uint8_t column, const uint8_t* data, uint8_t length);

#ifndef BOOTLOADER
#include <font.h>
void display_write_text(int page, int column, const char* text, const struct FONT *font, bool right_justify);

void display_write_multiline(int page,const char* text, const struct FONT *font);

void display_rle_image(const char image[]);

int render_text_to_page(uint8_t *buffer, int page, int column, const char *text, const struct FONT *font);

void display_scroll_page(uint8_t *data, bool up);

void display_swipe_pages(int start_page, uint8_t *data, int page_count, bool left);

void display_scroll_text(int page, int column, const char *text, const struct FONT *font, bool up);

void display_flip(bool invert);

void display_set_day(bool day);

#endif
#endif
