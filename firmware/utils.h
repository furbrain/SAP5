#ifndef UTILS_H
#define UTILS_H

#define SAFE_STRING_COPY(dest, src, count) \
    if (strlen(src) > count-1) Throw(ERROR_STRING_TOO_BIG);\
    strcpy(dest,src);


void delay_ms(int count);

void wdt_clear(void);

void sys_reset(int32_t a);

#endif
