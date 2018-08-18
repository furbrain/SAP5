#include <stdio.h>

#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "interface.h"
#include "laser.h"
#define TXT_LENGTH 50

void main(void)
{
    
    struct RAW_SENSORS sensors;
    char text[TXT_LENGTH];
    int i;
    double f;
    enum ACTION action;
    wdt_clear();
    SYSTEM_Initialize();
    wdt_clear();
    PERIPH_EN_SetHigh();
    TMR2_Start();
    wdt_clear();
    delay_ms(100);
    wdt_clear();
    display_init();
    wdt_clear();
    display_clear_screen();
    wdt_clear();
    display_write_text(3,1,"On",&large_font,false);
    wdt_clear();
    while (1)
    {
        wdt_clear();
        f = laser_read(LASER_MEDIUM, 500);
        wdt_clear();
        snprintf(text, TXT_LENGTH, "%06.3f", f);
        display_write_text(3,1,text,&large_font, false);
        wdt_clear();
        delay_ms(200);
    }
}
