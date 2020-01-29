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
    display_clear_screen(true);
    error = exception_get_string(e);
    exception_get_details(&reason, &file, &line);
    snprintf(text,18,"Err: %s", error);
    display_write_multiline(0,text, true);
    snprintf(text,18,"%s", reason);
    display_write_multiline(2,text, true);
    if (strlen(reason)>17) {
        snprintf(text,18,"%s", reason+16);
        display_write_multiline(4,text, true);        
    }
    snprintf(text,18,"%s:%d", file, line);
    display_write_multiline(6,text, true);
    delay_ms_safe(5000);
}

void initialise() {
    wdt_clear();
    RTCC_TimeReset(true);
    SYSTEM_Initialize();
    beep_initialise();
    PERIPH_EN_SetHigh();
    exception_init();
    memory_clear_errors();
    TMR2_Start();
    config_load();
    survey_current_init();
    delay_ms_safe(300);
    if (battery_get_voltage()<3.4) {
        beep_sad();
        utils_turn_off(0);
    }
    find_version();
    display_init();
    sensors_init();
    input_init();
    wdt_clear();
    display_clear_screen(true);
    if (battery_get_voltage() < 3.5) {
        display_write_text(0,0,"Low", &large_font, false, true);
        display_write_text(4,0,"Battery", &large_font, false, true);
        beep_sad();
        delay_ms_safe(1000);
        display_clear_screen(true);
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
