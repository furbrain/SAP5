#include <stdio.h>
#include <string.h>
#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "interface.h"
#include "laser.h"
#include "i2c_util.h"
#include "maths.h"
#define TXT_LENGTH 50

void main(void)
{
    wdt_clear();
    RTCC_TimeReset(true);
    SYSTEM_Initialize();
    PERIPH_EN_SetHigh();
    TMR2_Start();
    wdt_clear();
    config_init();
    delay_ms(100);
    wdt_clear();
    display_init();
    wdt_clear();
    display_clear_screen();
    wdt_clear();
    sensors_init();    
    wdt_clear();
    while (1)
    {
        show_menu(main_menu, FIRST_MENU_ITEM, true);
        wdt_clear();
    }
}
