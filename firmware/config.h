#ifndef CONFIG_H
#define CONFIG_H
#include "app_type.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>

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
    UNINITIALISED = 0,
    POLAR,
    GRAD,
    CARTESIAN
};

enum LENGTH_UNITS {
    METRIC,
    IMPERIAL
};

struct CONFIG {
    struct {
        uint8_t accel[3];
        uint8_t mag[3];
        
    } axes;
    struct {
        float accel[3][3];
        float mag[3][3];
        float laser_offset;
    } calib;
    uint8_t display_style;
    uint8_t length_units;
};

struct LEG {
    time_t dt;
    /* leg count */
    uint16_t number; //2 bytes
    /* differential readings, stored as multiples of 0.25cm */
    int16_t delta[3]; //6 bytes
    uint16_t pad; //2 bytes - pads out record to total of 16 bytes
};

#ifndef BOOTLOADER
extern struct CONFIG config;

extern uint16_t current_leg;

extern bool day;

void config_init();
void config_save();

void config_set_metric();
void config_set_imperial();
void config_set_cartesian();
void config_set_polar();
void config_set_grad();
void set_day();
void set_night();


#endif
#endif
