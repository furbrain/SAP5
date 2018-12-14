#include <stdio.h>
#include <string.h>
#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "survey.h"
#include "interface.h"
#include "laser.h"
#include "i2c_util.h"
#include "maths.h"
#include "exception.h"
#define TXT_LENGTH 50

void display_error(CEXCEPTION_T e) {
    char text[20];
    const char *error;
    const char *reason;
    const char *file;
    int line;
    display_on(true);
    laser_on(false);
    display_clear_screen();
    error = exception_get_string(e);
    exception_get_details(&reason, &file, &line);
    snprintf(text,18,"Err: %s", error);
    display_write_multiline(0,text, &small_font);
    snprintf(text,18,"%s", reason);
    display_write_multiline(2,text, &small_font);
    if (strlen(reason)>17) {
        snprintf(text,18,"%s", reason+16);
        display_write_multiline(4,text, &small_font);        
    }
    snprintf(text,18,"%s:%d", file, line);
    display_write_multiline(6,text, &small_font);
    delay_ms_safe(5000);
}

void main(void)
{
    CEXCEPTION_T e;
    wdt_clear();
    RTCC_TimeReset(true);
    SYSTEM_Initialize();
    PERIPH_EN_SetHigh();
    exception_init();
    TMR2_Start();
    config_init();
    survey_init();
    display_init();
    sensors_init();    
    wdt_clear();
    display_clear_screen();
    delay_ms_safe(10);
    while (1)
    {
        Try {
            show_menu(&main_menu);
            wdt_clear();
        }
        Catch(e) {
            display_error(e);
        }
    }
}
