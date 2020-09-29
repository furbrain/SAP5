/*
 * my_utils.c
 *
 *  Created on: 19 Sep 2020
 *      Author: phil
 */
#include <string.h>
#include "my_utils.h"

bool startsWith(const char* text, const char* prefix) {
	return (strncmp(text, prefix, strlen(prefix))==0);
}

void appendChar(char *s, const char c, int len) {
	int buf_len = strnlen(s, len);
	if (buf_len >= len-1) {
		buf_len = len - 2;
	}
	s[buf_len] = c;
	s[buf_len+1] = 0;
}

void clearStr(char *s) {
	s[0] = 0;
}

int getDigit(char c) {
	switch (c) {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'a':
	case 'A': return 10;
	case 'b':
	case 'B': return 11;
	case 'c':
	case 'C': return 12;
	case 'd':
	case 'D': return 13;
	case 'e':
	case 'E': return 14;
	case 'f':
	case 'F': return 15;
	default:
		return -1;
	}
}

int hexToChar(char *dest, const char *src, int maxlen) {
	int a, b;
	int i=0;
	while (*src) {
		a = getDigit(*src++);
		b = getDigit(*src++);
		if ((a<0) || (b<0)) return -1;
		dest[i++] = a*16+b;
	}
	return i;
}
