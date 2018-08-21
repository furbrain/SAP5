#ifndef SENSORS_H
#define SENSORS_H
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>
#include "config.h"
//#define LIDAR_TESTING

struct RAW_SENSORS {
    int16_t accel[3];
    int16_t temp;
    int16_t gyro[3];
    int16_t mag[3];
};

struct COOKED_SENSORS {
    accum accel[3];    //measured in g
    accum temp;        //measured in degs C
    accum gyro[3];     //measured in degress per second
    accum mag[3];      //measured in uT
};

void sensors_init();

void sensors_init_compass();

void sensors_read_raw(struct RAW_SENSORS *sensors, bool lidar);

void sensors_read_cooked(struct COOKED_SENSORS *sensors, bool lidar);

void sensors_raw_to_cooked(struct COOKED_SENSORS *cooked, struct RAW_SENSORS *raw);

void sensors_get_orientation(struct COOKED_SENSORS *sensors, accum *d);

extern volatile int lidar_average_count;

int32_t sensors_read_lidar();

void sensors_enable_lidar(bool on);
#endif
