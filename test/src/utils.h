#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include <time.h>

#ifdef TEST
#define TESTABLE_STATIC
#else
#define TESTABLE_STATIC static
#endif

enum FLASH_OP {
    FLASH_CLEAR_ERROR = 0x4000,
    FLASH_WRITE_DWORD = 0x4002,
    FLASH_WRITE_ROW = 0x4003,
    FLASH_ERASE_PAGE = 0x4004,
    FLASH_ERASE_CHIP = 0x4005
};    


#define SAFE_STRING_COPY(dest, src, count) \
    if (strlen(src) > count-1) Throw(ERROR_STRING_TOO_BIG);\
    strcpy(dest,src);

#ifndef BOOTLOADER
/* initialise beeping circuitry...*/
void bt_and_beep_initialise(void);

/* beep at freq for duration milliseconds*/
void beep(double freq, int duration);
#endif

void delay_ms(int count);

/* delay for count ms, performing wdt_clear every 500ms */
void delay_ms_safe(int count);

/* get current time in time_t format */
time_t utils_get_time(void);

void wdt_clear(void);

void utils_turn_off(int32_t a);

int utils_flash_memory (void *dest, const void *data, enum FLASH_OP op);

#endif
