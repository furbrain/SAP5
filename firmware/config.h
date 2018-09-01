#ifndef CONFIG_H
#define CONFIG_H
#include "app_type.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>
#include "maths.h"

#define FIRMWARE_VERSION 1


/* define sensor axes in reference to unit *
 * all references are imagined with the display facing uppermost and the laser pointing north *
 * x-axis is east
 * y-axis is north
 * z-axis is up
 * 
 * for the gyro the axis indicates the axis of rotation */

#define SWITCH_ACTIVE_HIGH false

enum DISPLAY_STYLE {
    POLAR = 0,
    GRAD,
    CARTESIAN
};

enum LENGTH_UNITS {
    METRIC = 0,
    IMPERIAL
};

struct __attribute__((aligned(8))) CONFIG {
    struct {
        uint8_t accel[3];
        uint8_t mag[3];
        
    } axes;
    struct {
        matrixx accel;
        matrixx mag;
        accum laser_offset;
    } calib;
    uint8_t display_style;
    uint8_t length_units;
    uint16_t timeout;
};
// this is currently 88 bytes long - perfect!
// note this needs to be padded to modulo 8 bytes to allow saving to work...

struct __attribute__((aligned(8))) LEG {
    time_t dt; // time of reading
    uint16_t survey; //survey number
    uint8_t from; //origin station
    uint8_t to; //destination station
    /* differential readings, stored as multiples of 0.25cm */
    accum delta[3]; 
};

#ifndef BOOTLOADER
extern struct CONFIG config;

extern uint16_t current_leg;

extern bool day;

void config_init();
void config_save();

void config_set_units(int32_t units);
void config_set_style(int32_t style);
void config_set_day(int32_t on);
void config_set_timeout(int32_t timeout);

#endif
#endif
