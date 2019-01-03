#include <stdbool.h>

#include "debug.h"
#include "interface.h"
#include "sensors.h"
#include "display.h"
#include "font.h"
#include "exception.h"
#include "battery.h"
#include "utils.h"


void show_sensors(int32_t a) {
    struct COOKED_SENSORS sensors;
    int i;
    char text[20];
    while (true) {
        switch (get_input()) {
            case SINGLE_CLICK:
            case DOUBLE_CLICK:
            case LONG_CLICK:
                return;
                break;
            default:
                break;
        }
        sensors_read_uncalibrated(&sensors);
        display_clear_screen(false);
        display_write_text(0, 0, "      Mag   Grav", &small_font, false, false);
        for(i=0; i<3; i++) {
            sprintf(text,"%c: %6.2f %6.2f", 'X'+i, sensors.mag[i], sensors.accel[i]);
            display_write_text(2+2*i, 0, text, &small_font, false, false);
        }        
        display_show_buffer();
        delay_ms_safe(500);
    }
}

void show_details(int32_t a) {
    struct COOKED_SENSORS sensors;
    char text[20];
    double voltage;
    while (true) {
        switch (get_input()) {
            case SINGLE_CLICK:
            case DOUBLE_CLICK:
            case LONG_CLICK:
                return;
                break;
            default:
                break;
        }
        sensors_read_uncalibrated(&sensors);
        voltage = battery_get_voltage();
        display_clear_screen(false);
        sprintf(text,"Voltage: %4.2fv", voltage);
        display_write_text(2, 0, text, &small_font, false, false);
        sprintf(text,"Temp: %4.1f`", sensors.temp);
        display_write_text(4, 0, text, &small_font, false, false);
        display_show_buffer();
        delay_ms_safe(500);
    }
}

void throw_error(int32_t a) {
    THROW_WITH_REASON("Just a random reason to get cross", ERROR_UNSPECIFIED);
}

void freeze_error(int32_t a) {
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    volatile int32_t x;
#pragma GCC diagnostic pop
    while(1) {x = a;};
}

void div_by_zero(int32_t a) {
    int i1=1;
    int i0=0;
    i0 = i1/i0;
}


DECLARE_MENU(debug_menu, {
    {"Sensors", Action, {show_sensors}, 0},
    {"Readings", Action, {show_details}, 0},
    {"Throw", Action, {throw_error}, 0},
    {"Freeze", Action, {freeze_error}, 0},
    {"DivByZero", Action, {div_by_zero}, 0}            
});
