#include <stdbool.h>
#include <time.h>
#include <xc.h>

#include "mcc_generated_files/rtcc.h"
#include "debug.h"
#include "interface.h"
#include "sensors.h"
#include "display.h"
#include "font.h"
#include "exception.h"
#include "battery.h"
#include "utils.h"
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"

void SetPlainFRC(void) {
    SYSTEM_RegUnlock();
    OSCCONbits.NOSC = 0;
    OSCCONSET = 1;
    SYSTEM_RegLock();
    while (OSCCONbits.OSWEN);
}

void DisableModules(void) {
    SYSTEM_RegUnlock();
    PMDCONbits.PMDLOCK = 0;
    PMD1 = 0xffffffff;
    PMD2 = 0xffffffff;
    PMD3 = 0xffffffff;
    PMD4 = 0xffffffff;
    PMD5 = 0xffffffff;
    PMD6 = 0xfffffffe; //leave RTCC on...
    PMD7 = 0xffffffff;
    PMDCONbits.PMDLOCK = 1;
    SYSTEM_RegLock();
    
}

void EnableModules(void) {
    SYSTEM_RegUnlock();
    PMDCONbits.PMDLOCK = 0;
    PMD1 = 0;
    PMD2 = 0;
    PMD3 = 0;
    PMD4 = 0;
    PMD5 = 0;
    PMD6 = 0; //leave RTCC on...
    PMD7 = 0;
    PMDCONbits.PMDLOCK = 1;
    SYSTEM_RegLock();
    
}

void sleep(void) {
    //TRISA = 0;
    //TRISB = 0;
    //TRISC = 0;
    TRISCbits.TRISC9 = 1;
    TRISBbits.TRISB6 = 1;
    SetPlainFRC();
    DisableModules();
    SYSTEM_RegUnlock();
    PWRCONbits.VREGS = 0;
    PWRCONbits.RETEN = 1;    
    OSCCONbits.SLPEN = 1;
    SYSTEM_RegLock();
    __builtin_enable_interrupts();
    asm("wait");
    asm("nop;nop;nop;nop;");
    EnableModules();
}

void do_sleep(int32_t a) {
    delay_ms_safe(1000);
    PIN_MANAGER_Initialize();
    PERIPH_EN_SetLow();
    TMR1_Stop();
    TMR2_Stop();
    wdt_clear();
    sleep();
    wdt_clear();
    TMR2_Start();

    PERIPH_EN_SetHigh();
    display_init();
    config.length_units=IMPERIAL;
}

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
        struct tm dt;
        RTCC_TimeGet(&dt);
        strftime(text, 20, "Time: %H:%M:%S", &dt);
        display_write_text(0, 0, text, &small_font, false, false);
        strftime(text, 20, "Date: %Y-%m-%d", &dt);
        display_write_text(2, 0, text, &small_font, false, false);
        sprintf(text,"Voltage: %4.2fv", voltage);
        display_write_text(4, 0, text, &small_font, false, false);
        sprintf(text,"Temp: %4.1f`", sensors.temp);
        display_write_text(6, 0, text, &small_font, false, false);
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
    {"Sleep", Action, {do_sleep}, 0},
    {"Sensors", Action, {show_sensors}, 0},
    {"Readings", Action, {show_details}, 0},
    {"Throw", Action, {throw_error}, 0},
    {"Freeze", Action, {freeze_error}, 0},
    {"DivByZero", Action, {div_by_zero}, 0}            
});
