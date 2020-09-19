/*
 * my_utils.h
 *
 *  Created on: 19 Sep 2020
 *      Author: phil
 */

#ifndef MY_UTILS_H_
#define MY_UTILS_H_
#include <stdbool.h>
#define BUF_SIZE 40

bool startsWith(const char* s, const char *prefix);
void appendChar(char *s, const char c, int len);
void clearStr(char *s);

#endif /* MY_UTILS_H_ */
