#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

enum FLASH_OP {
    FLASH_WRITE_DWORD = 0x4002,
    FLASH_WRITE_ROW = 0x4003,
    FLASH_ERASE_PAGE = 0x4004,
    FLASH_ERASE_CHIP = 0x4005
};    


#define SAFE_STRING_COPY(dest, src, count) \
    if (strlen(src) > count-1) Throw(ERROR_STRING_TOO_BIG);\
    strcpy(dest,src);


void delay_ms(int count);

/* delay for count ms, performing wdt_clear every 500ms */
void delay_ms_safe(int count);

void wdt_clear(void);

void sys_reset(int32_t a);

int utils_flash_memory (void *dest, const void *data, enum FLASH_OP op);

#endif
