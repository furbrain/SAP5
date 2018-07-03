#ifndef CONFIG_H
#define CONFIG_H
#define USE_AND_OR
#define FCY 16000000L
#define FCY_PER_MS 16000L //this is useful where working with 16-bit numbers
#include <libpic30.h>
#define delay_ms(delay) __delay_ms(delay)

#define FIRMWARE_VERSION 1

#define INT0_ACTIVE_HIGH 1

/* define sensor axes in reference to unit *
 * all references are imagined with the display facing uppermost and the laser pointing north *
 * x-axis is east
 * y-axis is north
 * z-axis is up
 * 
 * for the gyro the axis indicates the axis of rotation */
//#define TESTBED
#ifdef TESTBED
#define GX_AXIS 1
#define GX_POLARITY -1

#define GY_AXIS 0
#define GY_POLARITY +1

#define GZ_AXIS 2
#define GZ_POLARITY -1


#define MX_AXIS 0
#define MX_POLARITY +1

#define MY_AXIS 1
#define MY_POLARITY +1

#define MZ_AXIS 2
#define MZ_POLARITY +1


#define AX_AXIS 1
#define AX_POLARITY -1

#define AY_AXIS 0
#define AY_POLARITY +1

#define AZ_AXIS 2
#define AZ_POLARITY -1

#else

#define GX_AXIS 0
#define GX_POLARITY +1

#define GY_AXIS 1
#define GY_POLARITY +1

#define GZ_AXIS 2
#define GZ_POLARITY +1


#define MX_AXIS 1
#define MX_POLARITY -1

#define MY_AXIS 0
#define MY_POLARITY +1

#define MZ_AXIS 2
#define MZ_POLARITY -1


#define AX_AXIS 0
#define AX_POLARITY -1

#define AY_AXIS 1
#define AY_POLARITY -1

#define AZ_AXIS 2
#define AZ_POLARITY +1
#endif

#include <stdint.h>
#include <stdbool.h>

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
        float accel[3][3];
        float mag[3][3];
        float laser_offset;
    } calib;
    uint8_t display_style;
    uint8_t length_units;
};

struct LEG {
    /* date/time encoded in BCD format */
    struct {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    } dt;        //6 bytes
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

void set_metric();
void set_imperial();
void set_cartesian();
void set_polar();
void set_grad();
void set_day();
void set_night();

void write_eeprom(unsigned int address, void *buffer, unsigned int len);

#endif
#endif
