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
