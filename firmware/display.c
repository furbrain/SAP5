#include <string.h>
#include <stdlib.h>

#include "display.h"
#include "i2c_util.h"
#include "utils.h"

#define SCROLL_RATE 0
#define SWIPE_STEP 8
#define DISPLAY_ADDRESS 0x3c

#ifndef BOOTLOADER
uint8_t display_buffer[NUM_PAGES][128];
uint8_t display_secondary_buffer[NUM_PAGES][128];
#endif

bool display_inverted = true;
int cur_column = 0;
int cur_page = 0;

int8_t send1(uint8_t command) {
	return write_i2c_command_block(DISPLAY_ADDRESS,0x00,&command,1);
}

int8_t send2(uint8_t command, uint8_t data) {
	uint8_t both_bytes[2];
	both_bytes[0] = command;
	both_bytes[1] = data;
	return write_i2c_command_block(DISPLAY_ADDRESS,0x00,both_bytes,2);
}

void display_init() {
	//set up and power up display
	send1(0xAD);
	send1(0x8B);
	delay_ms(500);
	send2(0xA8,0x3F);
	send2(0xD5,0x50);
	send2(0xDA,0x12);
	send1(0xAF);
	send1(0xA1);
	send1(0xC8);
	display_set_brightness(0xFF);
	send2(0xD3,0);
	send1(0x40);
	send1(0xA6);
}

void display_close() {
	send1(0xAE);
}

void display_on(){
    send1(0xAF); 
}

void display_off() {
    send1 (0xAE);
}


void set_page(int page) {
	send1(page+0xB0);
	cur_page = page;
}

void set_column(int column) {
	send1((column+2) % 16);
	send1(16+((column+2)/16));
	cur_column = column;
}

void display_send_data(const uint8_t *data, uint8_t length) {
#ifndef BOOTLOADER
	if (length+cur_column<=128) { 
		memcpy(&(display_buffer[cur_page][cur_column]),data,length);
	}
#endif
	write_i2c_command_block(DISPLAY_ADDRESS,0x40,data,length);
}


void render_data_to_page(uint8_t page, uint8_t column, const uint8_t *data, uint8_t length) {
    set_page(page);
    set_column(column);
    display_send_data(data,length);
}


void display_clear_page(uint8_t page, bool immediate) {
	set_column(0);
	set_page(page);
#ifdef BOOTLOADER
	uint8_t display_buffer[128];
	memset(display_buffer,0,128);
	write_i2c_command_block(DISPLAY_ADDRESS,0x40,display_buffer,128);
#else
	memset(display_buffer[page],0,128);
    if (immediate) {
        write_i2c_command_block(DISPLAY_ADDRESS,0x40,display_buffer[page],128);
    }
#endif
}
void display_clear_screen(bool immediate) {
	int x = 0;
	for(x=0;x<8;++x) {
		display_clear_page(x, immediate);
	}
}

void display_set_brightness(uint8_t brightness){
	send2(0x81,brightness);
}

#ifndef BOOTLOADER
void display_write_text(int page, int column, const char* text, const struct FONT *font, bool right_justify, bool immediate) {
    int i = 0;
    int end_col;
    uint8_t temp_buffer[128];
    while (i<font->max_pages) {
        memset(temp_buffer,0,128);
        if (immediate) set_page(page+i);
        if (right_justify) {
            end_col = render_text_to_page(temp_buffer,i,0,text,font);
            if (immediate) {
                if (end_col > column) {
                    set_column(0);
                } else {
                    set_column(column-end_col);
                }
                display_send_data(temp_buffer,end_col);
            } else {
                if (end_col > column) {
                    render_text_to_page(display_buffer[page+i], i, 0, text, font);
                } else {
                    render_text_to_page(display_buffer[page+i], i, column-end_col, text, font);                    
                }
            }
        } else {
            if (immediate) {
                end_col = render_text_to_page(temp_buffer,i,column,text,font);
                set_column(column);
                display_send_data(&temp_buffer[column],end_col-column);
            } else {
                render_text_to_page(display_buffer[page+i],i, column, text, font);
            }
            
        }
        i++;
    }
}

void display_write_multiline(int page, const char* text, bool immediate) {
	char buf[18];
	int i = 0;
	while (*text) {
		if (*text=='\n') {
            buf[i] = 0;
            display_write_text(page, 0, buf, &small_font, false, immediate);
            i = 0;
            page+=2;
		} else {
			if (i<17) {
				buf[i] = *text;
				i++;
			}
		}
		text++;
	}
    buf[i]=0;
	display_write_text(page, 0, buf, &small_font, false, immediate);
}

/* display an rle encoded image */
/* each line starts with a blank pixel */
void display_rle_image(const char* image) {
	int page = 0;
	int row = 0;
	int image_counter;
	int column = 0;
	int colour = 1;
	while (page<8) {
		image_counter = *image;
		image++;
		colour ^= 1;
		while (image_counter--) {
			if (colour) {
				display_buffer[page][column] |= 1<<row;
			} else {
				display_buffer[page][column] &= ~(1<<row);
			}
			column++;
			if (column>=128) {
				column = 0;
				row++;
				colour = 1;
				if (row>=8) {
					page++;
					row=0;
				}
				break;
			}
		}
	}
	for (page=0; page<8; ++page) {
	    set_column(0);
	    set_page(page);
	    display_send_data(display_buffer[page],128);
	}

	
}


/* scroll a page of data onto the screen either up or down */
void display_scroll_page(uint8_t *data,  bool up){
	int i,j;
	uint8_t mask;
	uint8_t temp_buffer[128];
        //first clear all data
	memset(temp_buffer,0,128);
    if (up) {
        set_page(top_page);
        set_column(0);
        display_send_data(temp_buffer,128);
        // now rotate it
        for(i=0;i<8;++i) {
            send1(0x40 | (top_page*NUM_PAGES+i+1) % 64);
	        set_column(0);
	        mask = ((1<<(i+1))-1);
	        for(j=0;j<128;++j){
		        temp_buffer[j] = data[j] & mask;
	        }
	        display_send_data(temp_buffer,128);
	        delay_ms(SCROLL_RATE);
        }
        top_page = (top_page+1) % NUM_PAGES;
	} else {
        set_page((top_page+NUM_PAGES-1)%NUM_PAGES);
        set_column(0);
        display_send_data(temp_buffer,128);
        // now rotate it
	    for(i=0;i<8;++i){
            send1(0x40 | (top_page*NUM_PAGES-(i+1)) % 64);
	        set_column(0);
	        mask = (256-(1<<(NUM_PAGES-i)));
	        for(j=0;j<128;++j){
		        temp_buffer[j] = data[j] & mask;
	        }
	        display_send_data(temp_buffer,128);
            delay_ms(SCROLL_RATE);
	    }
        top_page = (top_page+NUM_PAGES-1) % NUM_PAGES;
	    }
	set_column(0);
	display_send_data(data,128);
}

/* render a text string to a buffer */ 
/* just do one page at a time */
/* can specify starting column */

int render_text_to_page(uint8_t *buffer, int page, int column, const char *text, const struct FONT *font) {
    int i;
    const struct GLYPH_DATA *glyph;
    while (*text) {
        i = 0;
        glyph = get_glyph_data(font,*text);        
        while((column<128) && (i < glyph->width)) {
            if (page < glyph->pages) {
                buffer[column] = font->bitmaps[glyph->index+page*glyph->width+i];
            } else {
                buffer[column] = 0;
            }
            column++;
            i++;
        }
        i = 0;
        while((column<128) && (i < font->advance)) {
            buffer[column] = 0;
            column++;
            i++;
        }
        text++;
    }
    return column;
}

/* scroll in a piece of text, finishing at page and column specified*/
void display_scroll_buffer(display_buf_t data, bool up) {
    int i;
    if (up) {
        for (i=0; i< 8; i++) {
            display_scroll_page(data[i], up);
        }
    } else {
        for (i=7; i>=0; i--) {
            display_scroll_page(data[i], up);
        }
    }
}

void display_swipe_pages(int start_page, display_buf_t data, int page_count, bool left){
    int offset;
    int page, real_page;
    uint8_t temp_buffer[128];
    for(offset=0;offset<128;offset+=SWIPE_STEP){
        for(page=start_page; page < start_page + page_count; ++page){
            memset(temp_buffer,0,128);
            // copy from current buffer over into temp_buffer, 
           if (left){
                memcpy(temp_buffer, &display_buffer[page][SWIPE_STEP], 128-SWIPE_STEP-offset);
                memcpy(&temp_buffer[128-SWIPE_STEP-offset], &data[page][0], offset);
            } else {
                memcpy(&temp_buffer[SWIPE_STEP], &display_buffer[page][0], 128-SWIPE_STEP);
                memcpy(temp_buffer, &data[page][128-SWIPE_STEP-offset], SWIPE_STEP);
            }
            set_page(page);
            set_column(0);
            display_send_data(temp_buffer,128);
        }
        delay_ms(SCROLL_RATE);
    }
}

void display_flip(bool invert) {
	if (invert) {
		send1(0xA1);
		send1(0xC8);
	} else {
		send1(0xA0);
		send1(0xC0);
	}
	display_inverted = invert;
	display_show_buffer();	
}

void display_set_day(bool day) {
    if (day) {
        send2(0xA8,0x1F);
        send2(0xD3,0x30);
    } else {
       	send2(0xA8,0x3F);
        send2(0xD3,0x00);
    }
}

void display_show_buffer(void) {
    int i;
	for (i=0; i<8; ++i) {
	    set_column(0);
	    set_page(i);
	    display_send_data(display_buffer[i],128);
	}
}

void display_setbuffer_xy(int x, int y) {
	int page;
    if (x<0) return;
    if (y<0) return;
    if (x>=DISPLAY_WIDTH) return;
    if (y>=DISPLAY_HEIGHT) return;
	page = (y/8);
	display_buffer[page][x] |= 1 << (y%8);
}

void display_clearbuffer_xy(int x, int y) {
	int page;
    if (x<0) return;
    if (y<0) return;
    if (x>=DISPLAY_WIDTH) return;
    if (y>=DISPLAY_HEIGHT) return;
	page = (y/8);
	display_buffer[page][x] &= ~(1 << (y%8));	
}

void display_draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    while (true) {
        display_setbuffer_xy(x0,y0);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }    
}
#endif
