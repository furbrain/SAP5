#ifndef SENSORS_H
#define SENSORS_H
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>
#include "config.h"
#include "maths.h"
//#define LIDAR_TESTING

struct RAW_SENSORS {
    int16_t accel[3];
    int16_t temp;
    int16_t gyro[3];
    int16_t mag[3];
};

struct COOKED_SENSORS {
    vectorr accel;    //measured in g
    double temp;        //measured in degs C
    vectorr gyro;     //measured in degress per second
    vectorr mag;      //measured in uT
};

void sensors_init();

void sensors_read_raw(struct RAW_SENSORS *sensors);

void sensors_read_uncalibrated(struct COOKED_SENSORS *sensors);

void sensors_read_cooked(struct COOKED_SENSORS *sensors);

void sensors_uncalibrated_to_cooked(struct COOKED_SENSORS *sensors);

void sensors_get_orientation(struct COOKED_SENSORS *sensors, double *d);
#endif
