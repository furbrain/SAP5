#ifndef _SELECTOR_H
#define _SELECTOR_H
#include <stdint.h>

struct SELECTOR_CHOICES {
    char *text; //pointer to an array of single chars
    int start; // first allowed char
    int end; // last allowed char
    int current;
};

/* write a single character for a selector at column*/
void selector_write_char(char text, int column);

/* display a selector at given page and column
 * the char will be displayed at page 2, column in large font
 * up and down markers will be displayed above it *
 * return the character selected */
char selector_choose(struct SELECTOR_CHOICES *choices, int column);

#ifdef TEST
extern const uint8_t up_marker[15];
extern const uint8_t down_marker[15];
extern const uint8_t empty_marker[15];
#endif
#endif // _SELECTOR_H
