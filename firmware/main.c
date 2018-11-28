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

void store_readings(void) {
    int i, j;
    struct COOKED_SENSORS sensors;
    struct LEG leg;
    display_on(false);
    delay_ms(100);
    wdt_clear();
    for(i=0; i<100; i++) {
        sensors_read_uncalibrated(&sensors);
        for(j=0; j<3; j++) {
            leg.delta[j] = sensors.mag[j];
        }
        leg.survey = i+256;
        write_leg(&leg);
        delay_ms(10);
        wdt_clear();
    }
    laser_on();
    delay_ms(100);
    wdt_clear();
    for(i=0; i<100; i++) {
        sensors_read_uncalibrated(&sensors);
        for(j=0; j<3; j++) {
            leg.delta[j] = sensors.mag[j];
        }
        leg.survey = i+256;
        write_leg(&leg);
        delay_ms(10);
        wdt_clear();
    }
    display_on(true);
    laser_off();
}

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
    delay_ms(100);
    wdt_clear();
    //display_write_text(2, 0, "Working", &large_font, false);
    //store_readings();
    while (1)
    {
        show_menu(&main_menu);
        wdt_clear();
    }
}
