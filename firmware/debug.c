#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <xc.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>

#include "mcc_generated_files/rtcc.h"
#include "debug.h"
#include "input.h"
#include "sensors.h"
#include "display.h"
#include "font.h"
#include "exception.h"
#include "battery.h"
#include "utils.h"
#include "gsl_static.h"
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/tmr2.h"
#include "measure.h"
#include "laser.h"
#include "leg.h"
#include "beep.h"
#include "memory.h"
#include "version.h"

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

void show_raw_sensors(int32_t a) {
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
        sensors_read_uncalibrated(&sensors, SAMPLES_PER_READING);
        display_clear_screen(false);
        display_write_text(0, 0, "      Mag   Grav", &small_font, false, false);
        for(i=0; i<3; i++) {
            sprintf(text,"%c: %6.2f %6.2f", 'X'+i, sensors.mag[i], sensors.accel[i]);
            display_write_text(2+2*i, 0, text, &small_font, false, false);
        }        
        display_show_buffer();
        delay_ms_safe(100);
    }
}

void show_calibrated_sensors(int32_t a) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag = gsl_vector_view_array(sensors.mag, 3);
    gsl_vector_view grav = gsl_vector_view_array(sensors.accel, 3);
    int i;
    char text[20];
    double M, g;
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
        sensors_read_cooked(&sensors, SAMPLES_PER_READING);
        display_clear_screen(false);
        M = gsl_blas_dnrm2(&mag.vector);
        M /= config.calib.mag[0];
        g /= config.calib.accel[0];
        g = gsl_blas_dnrm2(&grav.vector);
        display_write_text(0, 0, "      Mag   Grav", &small_font, false, false);
        sprintf(text, "M/g:%6.2f %6.2f", M, g);
        display_write_text(0,0, text, &small_font, false, false);
        for(i=0; i<3; i++) {
            sprintf(text,"%c:  %6.2f %6.2f", 'X'+i, sensors.mag[i], sensors.accel[i]);
            display_write_text(2+2*i, 0, text, &small_font, false, false);
        }        
        display_show_buffer();
        delay_ms_safe(100);
    }
}

void show_details(int32_t a) {
    struct COOKED_SENSORS sensors;
    char text[20];
    double voltage;
    while (true) {
        switch (get_input()) {
            case SINGLE_CLICK:
                return;
                break;
            case LONG_CLICK:
            case DOUBLE_CLICK:
            default:
                break;
        }
        sensors_read_uncalibrated(&sensors, 3);
        voltage = battery_get_voltage();
        display_clear_screen(false);
        struct tm dt;
        RTCC_TimeGet(&dt);
        strftime(text, 20, "Time: %H:%M:%S", &dt);
        display_write_text(0, 0, text, &small_font, false, false);
        strftime(text, 20, "Date: %Y-%m-%d", &dt);
        display_write_text(2, 0, text, &small_font, false, false);
        sprintf(text,"Battery: %4.2fv", voltage);
        display_write_text(4, 0, text, &small_font, false, false);
        sprintf(text,"Temp: %4.1f`", sensors.temp);
        display_write_text(6, 0, text, &small_font, false, false);
        display_show_buffer();
        delay_ms_safe(500);
    }
}

void show_bearings(int32_t a) {
    char stext[80];
    GSL_VECTOR_DECLARE(orientation, 3);
    double compass, inclination;
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
        sensors_get_orientation(&orientation, SAMPLES_PER_READING);
        measure_calculate_bearings(&orientation, &compass, &inclination);
        sprintf(stext,"Compass: %5.1f`\n  Clino: %+.1f`", compass, inclination);
        display_write_multiline(2, stext, true);
    }
}

void show_version(int32_t a) {
    char text[20];
    display_clear_screen(true);
    display_write_text(0, 0, "Hardware", &small_font, false, false);
    switch (version_hardware) {
        case VERSION_ALPHA:
            strcpy(text, "Alpha");
            break;
        case VERSION_V1_0:
            strcpy(text, "v1.0");
            break;
        case VERSION_V1_1:
            strcpy(text, "v1.1");
            break;
        default:
            strcpy(text, "Unknown");
            break;
    }
    display_write_text(0, 128, text, &small_font, true, false);
    sprintf(text, "%hhu.%hhu.%hhu",
            version_software.version.major,
            version_software.version.minor,
            version_software.version.revision);
    display_write_text(2,0,"Firmware", &small_font, false, false);
    display_write_text(2,128,text, &small_font, true, false);
    sprintf(text, "Data formats:\n %hhu, %hhu, %hhu", 
            version_software.config.version,
            version_software.legs.version,
            version_software.calibration.version);
    display_write_multiline(4, text, false);
    display_show_buffer();
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
        delay_ms_safe(10);
    }
}

void show_magnetism(int32_t a) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag = gsl_vector_view_array(sensors.mag, 3);
    gsl_vector_view grav = gsl_vector_view_array(sensors.accel, 3);
    char text[40];
    double M, g, dot, theta;
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
        sensors_read_cooked(&sensors, SAMPLES_PER_READING);
        M = gsl_blas_dnrm2(&mag.vector);
        g = gsl_blas_dnrm2(&grav.vector);
        gsl_blas_ddot(&mag.vector, &grav.vector, &dot);
        theta = acos(dot / M * g) * 360.0 / M_PI;
        M /= config.calib.mag[0];
        sprintf(text, "Field: %6.2f<T\nDip:   %5.1f`", M, theta);
        display_clear_screen(false);
        display_write_multiline(2, text, false);
        display_show_buffer();
        delay_ms_safe(100);
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

void test_battery(int32_t a) {
    GSL_VECTOR_DECLARE(orientation, 3);
    char text[80];
    struct voltage_record {
        int count;
        double voltage;
    };
    struct voltage_record record[32];
    int i;
    double voltage;
    CEXCEPTION_T e;
    display_clear_screen(true);
    memory_erase_page(leg_store.raw);
    memory_erase_page(&leg_store.raw[0x800]);
    memory_erase_page(&leg_store.raw[0x1000]);
    i = 0;
    do  {
        delay_ms_safe(15000);
        sensors_get_orientation(&orientation, SAMPLES_PER_READING);
        laser_on();
        Try {
            delay_ms_safe(2000);
            laser_read(LASER_MEDIUM, 3000);
        } Catch (e) {
            beep_sad();
        }
        laser_off();
        voltage = battery_get_voltage();
        record[i%32].voltage = voltage;
        record[i%32].count = i;
        sprintf(text, "Filler text here\nVoltage: %4.2f\n  Count: %d\nFiller text here", record[i%32].voltage, record[i%32].count);
        beep_beep();
        i++;
        if (i%32 == 0) {
            memory_write_data(&leg_store.raw[(i-32)*8], &record[0], 0x100);
        }
        PERIPH_EN_SetLow();
        delay_ms_safe(2000); //restart peripherals
        PERIPH_EN_SetHigh();
        delay_ms_safe(500);
        display_init();
        sensors_init();
        display_clear_screen(true);    
        display_write_multiline(0, text, true);
    } while (voltage > 3.4);
    utils_turn_off(0);
}


DECLARE_MENU(debug_menu, {
//    {"Sleep", Action, {do_sleep}, 0},
    {"Raw", Action, {show_raw_sensors}, 0},
    {"Calibrated", Action, {show_calibrated_sensors}, 0},
    {"Bearings", Action, {show_bearings}, 0},
    {"Magnetism", Action, {show_magnetism}, 0},
    {"Misc", Action, {show_details}, 0},
    {"Version", Action, {show_version}, 0},
//    {"Battery", Action, {test_battery}, 0},
//    {"Throw", Action, {throw_error}, 0},
//    {"Freeze", Action, {freeze_error}, 0},
//    {"DivByZero", Action, {div_by_zero}, 0}            
});
