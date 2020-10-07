#include <stdio.h>
#include <string.h>
#include "mcc_generated_files/mcc.h"
#include "measure.h"
#include "utils.h"
#include "display.h"
#include "sensors.h"
#include "survey.h"
#include "laser.h"
#include "exception.h"
#include "config.h"
#include "interface.h"
#include "input.h"
#include "memory.h"
//FIXME
#include "beep.h"
#include "battery.h"
#include "version.h"
#define TXT_LENGTH 50

void double_click_start(void) {
    int i=0;
    while (!SWITCH_GetValue()) {
        delay_ms_safe(10);
        }
    while (i++<50) {
        delay_ms(10);
        if (!SWITCH_GetValue()) {
            return;
        }
    }
    //no double_click in relevant time....
    utils_turn_off(0);
}

void display_error(CEXCEPTION_T e) {
    char text[20];
    const char *error;
    const char *reason;
    const char *file;
    int line;
    PERIPH_EN_SetHigh();
    delay_ms_safe(100);
    display_init();
    display_on();
    laser_off();
    display_clear(false);
    error = exception_get_string(e);
    exception_get_details(&reason, &file, &line);
    snprintf(text,18,"Err: %s", error);
    display_write_multiline(0,text, false);
    snprintf(text,18,"%s", reason);
    display_write_multiline(2,text, false);
    if (strlen(reason)>17) {
        snprintf(text,18,"%s", reason+16);
        display_write_multiline(4,text, false);        
    }
    snprintf(text,18,"%s:%d", file, line);
    display_write_multiline(6,text, false);
    display_show_buffer();
    delay_ms_safe(5000);
}

void initialise(void) {
    wdt_clear();
    RTCC_TimeReset(true);
    SYSTEM_Initialize();
    exception_init();
    memory_clear_errors();
    config_load();
    survey_current_init();
    delay_ms_safe(3);
    double_click_start();
    PERIPH_EN_SetHigh();
    input_init();
    delay_ms_safe(100);
    bt_and_beep_initialise();
    if (battery_get_voltage()<3.4) {
        beep_sad();
        utils_turn_off(0);
    }
    find_version();
    display_init();
    sensors_init();
    wdt_clear();
    display_clear(true);
    if (battery_get_voltage() < 3.5) {
        display_clear(false);
        display_write_text(0,0,"Low", &large_font, false);
        display_write_text(4,0,"Battery", &large_font, false);
        display_show_buffer();
        beep_sad();
        delay_ms_safe(1000);
        display_clear(true);
    }
}

int main(void)
{
    int err_count = 0;
    CEXCEPTION_T e;
    initialise();
    beep_happy();
    while (err_count < 3)
    {
        Try {
            measure();
        }
        Catch(e) {
            err_count++;
            display_error(e);
            beep_sad();
        }
    }
    utils_turn_off(0);
    return 0;
}
