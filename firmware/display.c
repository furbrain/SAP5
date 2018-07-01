#include "display.h"
#include "i2c_util.h"
#include <string.h>

#define SCROLL_RATE 0
#define SWIPE_STEP 8
#define DISPLAY_ADDRESS 0x3c
#define NUM_PAGES 8

#ifndef BOOTLOADER
uint8_t buffer[NUM_PAGES][128];
#endif

int top_page = 0;
bool display_inverted = false;
int cur_column = 0;
int cur_page = 0;

int8_t send1(uint8_t command) {
	write_i2c_command_block(DISPLAY_ADDRESS,0x00,&command,1,I2C_FAST);
}

int8_t send2(uint8_t command, uint8_t data) {
	uint8_t both_bytes[2];
	both_bytes[0] = command;
	both_bytes[1] = data;
	return write_i2c_command_block(DISPLAY_ADDRESS,0x00,both_bytes,2,I2C_FAST);
}

void display_init() {
	//set up and power up display
	send1(0xAD);
	send1(0x8B);
	__delay_ms(500);
	send2(0xA8,0x3F);
	send2(0xD5,0x50);
	send2(0xDA,0x12);
	send1(0xAF);
	send1(0xA0);
	send1(0xC0);
	display_set_brightness(0xFF);
	send2(0xD3,0);
	send1(0x40);
	send1(0xA6);
}

void display_close() {
	send1(0xAE);
}

void display_on(bool enable){
	if (enable) {
		send1(0xAF); 
	} else {
		send1 (0xAE);
	}
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
		memcpy(&(buffer[cur_page][cur_column]),data,length);
	}
#endif
	write_i2c_command_block(DISPLAY_ADDRESS,0x40,data,length,I2C_FAST);
}

void render_data_to_page(uint8_t page, uint8_t column, const uint8_t *data, uint8_t length) {
    page = (page+top_page)%8;
    set_page(page);
    set_column(column);
    display_send_data(data,length);
}


void display_clear_page(uint8_t page) {
	page = (page+top_page)%8;
	set_column(0);
	set_page(page);
#ifdef BOOTLOADER
	uint8_t buffer[128];
	memset(buffer,0,128);
	write_i2c_command_block(DISPLAY_ADDRESS,0x40,buffer,128,I2C_FAST);
#else
	memset(buffer[page],0,128);
	write_i2c_command_block(DISPLAY_ADDRESS,0x40,buffer[page],128,I2C_FAST);
#endif
}
void display_clear_screen() {
	int x = 0;
	for(x=0;x<8;++x) {
		display_clear_page(x);
	}
}

void display_set_brightness(uint8_t brightness){
	send2(0x81,brightness);
}

#ifndef BOOTLOADER
void display_write_text(int page, int column, const char* text, const struct FONT *font, bool right_justify) {
    int i = 0;
    int end_col;
    uint8_t temp_buffer[128];
    page = (page+top_page)%8;
    while (i<font->max_pages) {
        memset(temp_buffer,0,128);
        set_page(page+i);
        if (right_justify) {
            end_col = render_text_to_page(temp_buffer,i,0,text,font);
            if (end_col > column) {
                set_column(0);
            } else {
                set_column(column-end_col);
            }
                display_send_data(temp_buffer,end_col);
        } else {
            end_col = render_text_to_page(temp_buffer,i,column,text,font);
            set_column(column);
            display_send_data(&temp_buffer[column],end_col-column);
        }
        i++;
    }
}

void display_write_multiline(int page,const char* text, const struct FONT *font) {
	char buf[17];
	int i = 0;
	while (text) {
		if (*text=='\n') {
			if (i<17) {
				buf[i] = 0;
				display_write_text(page,0,buf,font,false);
				i = 0;
				page+=2;
			}
		} else {
			if (i<17) {
				buf[i] = *text;
				i++;
			}
		}
		text++;
	}
	display_write_text(page,0,buf,font,false);
}

/* display an rle encoded image */
/* each line starts with a blank pixel */
void display_rle_image(const char* image) {
	int page = 0;
	int row = 0;
	int image_counter,real_page;
	int row_counter = 0;
	int colour = 1;
	while (page<8) {
		image_counter = *image;
		image++;
		colour ^= 1;
		while (image_counter--) {
			real_page = (page+top_page)%8;
			if (colour) {
				buffer[real_page][row_counter] |= 1<<row;
			} else {
				buffer[real_page][row_counter] &= ~(1<<row);
			}
			row_counter++;
			if (row_counter>=128) {
				row_counter = 0;
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
	    display_send_data(buffer[page],128);
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
	        __delay_ms(SCROLL_RATE);
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
            __delay_ms(SCROLL_RATE);
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
            }
            column++;
            i++;
        }
        column+=font->advance;
        text++;
    }
    return column;
}

/* scroll in a piece of text, finishing at page and column specified*/
void display_scroll_text(int page, int column, const char *text, const struct FONT *font, bool up) {
    uint8_t temp_buffer[128];
    int i;
    if (up) {
        i = 0;
        while ((8-i)>page) {
            memset(temp_buffer,0,128);
            if (i < font->max_pages) render_text_to_page(temp_buffer,i,column,text,font);
            display_scroll_page(temp_buffer,up);
            i++;
        }
    } else {
        for(i=(font->max_pages-1); i > -1; i--) {
            memset(temp_buffer,0,128);
            render_text_to_page(temp_buffer,i,column,text,font);
            display_scroll_page(temp_buffer,up);
        }
        memset(temp_buffer,0,128);
        for(i=0; i < page; i++) {
            display_scroll_page(temp_buffer,up);
        }
    }
}

void display_swipe_pages(int start_page, uint8_t *data, int page_count, bool left){
    int offset;
    int page, real_page;
    uint8_t temp_buffer[128];
   if (left){
        for(offset=0;offset<128;offset+=SWIPE_STEP){
            for(page=0;page<page_count;++page){
                memset(temp_buffer,0,128);
                real_page = (start_page+top_page+page) % 8;
                // copy from current buffer over into temp_buffer, 
                memcpy(temp_buffer,&buffer[real_page][SWIPE_STEP],128-SWIPE_STEP-offset);
                memcpy(&temp_buffer[128-SWIPE_STEP-offset],&data[page*128],offset);
                set_page(real_page);
                set_column(0);
                display_send_data(temp_buffer,128);
            }
            __delay_ms(SCROLL_RATE);
        }
    } else {
        for(offset=0;offset<128;offset+=SWIPE_STEP){
            for(page=0;page<page_count;++page){
                memset(temp_buffer,0,128);
                real_page = (start_page+top_page+page) % 8;
                // copy from current buffer over into temp_buffer, 
                memcpy(&temp_buffer[SWIPE_STEP],&buffer[real_page][0],128-SWIPE_STEP);
                memcpy(temp_buffer,&data[page*128+128-SWIPE_STEP-offset],SWIPE_STEP);
                set_page(real_page);
                set_column(0);
                display_send_data(temp_buffer,128);
            }
            __delay_ms(SCROLL_RATE);
        }
    }
};

void display_flip(bool invert) {
    int i;
	if (invert) {
		send1(0xA1);
		send1(0xC8);
	} else {
		send1(0xA0);
		send1(0xC0);
	}
	display_inverted = invert;
	for (i=0; i<8; ++i) {
	    set_column(0);
	    set_page(i);
	    display_send_data(buffer[i],128);
	}
	
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

#endif
