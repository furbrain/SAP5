#define USE_AND_OR
#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"
#include "input.h"
#include "utils.h"
#include "display.h"
#include "mcc_generated_files/tmr2.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/interrupt_manager.h"

static volatile enum INPUT last_click;

/* set up timer interrupts etc */
/* Timer 2 is our input poller counter */
/* timer 3 is click length counter */
/* timer 2 delay: 2ms  = */
static uint32_t last_activity_counter;
static bool display_inverted;
static uint32_t button_click_counter;

/* setup input detection*/
void input_init(void) {
    INTERRUPT_GlobalDisable();
    last_click = NONE;
    last_activity_counter = 0;
    button_click_counter=10000;
    INTERRUPT_GlobalEnable();
}


/* change notification interrupt, called every 2ms*/
/* 0 = button pressed, 1 is released*/
void TMR2_CallBack(void) {
    static uint16_t state = 0x0001;
    static bool ignore_release = true;
    last_activity_counter++;
    button_click_counter++;
    
    state = ((state << 1) | SWITCH_GetValue()) & 0xffff;
    if (state == 0x8000) {
        /* we have just pressed the button and held it for 15 T2 cycles*/
        if (button_click_counter < 100) {
            /* it's been less than 0.2s since the last press finished */
            last_click = DOUBLE_CLICK;
            ignore_release = true;
        }
        last_activity_counter = 0;
    }
    if (state == 0x7fff) {
        /* we have just released the button*/
        last_activity_counter = 0;
        button_click_counter = 0;
        if (!ignore_release) {
            last_click = SINGLE_CLICK;
        }
        ignore_release = false;
    }
    if (state == 0x0000) {
        if (last_activity_counter > 750) {
            if (!ignore_release) {
                last_click = LONG_CLICK;
                ignore_release=true;
            }
        }        
    }
    if (last_activity_counter>(config.timeout*500)) {
        utils_turn_off(0);
    }
}

void timeout_reset() {
    last_activity_counter = 0;
}

enum INPUT get_input() {
    struct COOKED_SENSORS sensors;
    int i;
    sensors_read_cooked(&sensors, 3);
    /* look for "flip" movements */
    //debug("f%.2g",sensors.gyro[1]);
    for (i=0; i<3; i++) {
        if (abs(sensors.accel[i]> 1.6)) {
            last_activity_counter = 0;
            return SHAKE;
        }
    }
    if (sensors.gyro[1] < -30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_DOWN : FLIP_UP;
    }
    if (sensors.gyro[1] > 30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_UP : FLIP_DOWN;
    }
    if (sensors.gyro[0] > 30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_LEFT : FLIP_RIGHT;
    }
    if (sensors.gyro[0]<-30.0) {
        last_activity_counter = 0;
        return display_inverted ? FLIP_RIGHT : FLIP_LEFT;
    }
    /* check to see if display needs flipping */
    /* use 0.5g - gives a hysteresis of about +/- 30 degrees */
    if ((sensors.accel[0] > 0.5) && display_inverted) {
        display_flip(false);
        display_inverted = false;
    }
    if ((sensors.accel[0] < -0.5) && !display_inverted) {
        display_flip(true);
        display_inverted = true;
    }
    /* search for a click */
    return get_clicks();
}

enum INPUT get_clicks() {
    enum INPUT temp;
    if (last_click != NONE) {
        /* momentarily disable interrupts */
        INTERRUPT_GlobalDisable();
        temp = last_click;
        last_click = NONE;
        INTERRUPT_GlobalEnable();
        return temp;
    }
    //nothing else found - so return NONE
    return NONE;
    
}