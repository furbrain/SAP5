#ifndef CONFIG_H
#define CONFIG_H
#include "app_type.h"
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>
#include "maths.h"
#include "memory.h"
#include "mem_locations.h"

#define FIRMWARE_VERSION 1

#define MAX_CONFIG_COUNT (APP_CONFIG_SIZE / sizeof(struct CONFIG))

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
        double accel[12];
        double mag[12];
        double laser_offset;
    } calib;
    uint8_t display_style;
    uint8_t length_units;
    uint16_t timeout;
    bool bt_present;
};

/* Note it is important to keep this as a union to ensure that no other data/ program 
 * code is kepts in the gap between the last config and the top of its page in memory */
union CONFIG_STORE {
    CONST_STORE uint8_t raw[APP_CONFIG_SIZE];
    CONST_STORE struct CONFIG configs[MAX_CONFIG_COUNT];
};

extern union CONFIG_STORE config_store;



#ifndef BOOTLOADER
extern struct CONFIG config;

extern uint16_t current_leg;

extern bool day;

void config_save(void);
void config_load(void);

void config_set_units(int32_t units);
void config_set_style(int32_t style);
void config_set_day(int32_t on);
void config_set_timeout(int32_t timeout);

#endif
#ifdef TEST
extern const struct CONFIG default_config;
#endif

#endif
