#ifndef FONT_H
#define FONT_H
#include <stdint.h>

struct GLYPH_DATA {
    uint8_t  width;
    uint8_t  pages;
    uint16_t index;  
};

#define get_glyph_data(font,char) &(font->indices[char-32])

struct FONT {
    uint8_t advance; /* how much padding between characters (0 for monospace fonts) */
    uint8_t max_pages;
    uint8_t max_width;
    const struct GLYPH_DATA *indices;
    const char *bitmaps;
};
/* a list of font datas */
/* starts at a relative offset of 32 (so first character is space) */
/* ` (backtick) has been replaced by ° (degree)*/
extern const struct FONT large_font; 
extern const struct FONT small_font; 
#endif
