#ifndef SENSORS_H
#define SENSORS_H
#include <stdint.h>
#include <stdbool.h>
//#define LIDAR_TESTING

struct RAW_SENSORS {
    int16_t accel[3];
    int16_t temp;
    int16_t gyro[3];
    int16_t mag[3];
    int16_t distance;
};

struct COOKED_SENSORS {
    double accel[3];    //measured in g
    double temp;        //measured in degs C
    double gyro[3];     //measured in degress per second
    double mag[3];      //measured in uT
    double distance;    //measured in m
};

void sensors_init();

void sensors_init_compass();

void sensors_read_raw(struct RAW_SENSORS *sensors, bool lidar);

void sensors_read_cooked(struct COOKED_SENSORS *sensors, bool lidar);

void sensors_raw_to_cooked(struct COOKED_SENSORS *cooked, struct RAW_SENSORS *raw);

void sensors_get_orientation(struct COOKED_SENSORS *sensors,double *d);

extern volatile int lidar_average_count;

int32_t sensors_read_lidar();

void sensors_read_leg(struct LEG *leg, double *distance);

void sensors_enable_lidar(bool on);
#endif
