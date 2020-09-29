#ifndef SENSORS_H
#define SENSORS_H
#include <stdint.h>
#include <stdbool.h>
#include <stdfix.h>
#include "config.h"
#include "maths.h"
//#define LIDAR_TESTING

#define SAMPLES_PER_READING 50

struct RAW_SENSORS {
    int16_t accel[3];
    int16_t temp;
    int16_t gyro[3];
    int16_t mag[3];
};

struct COOKED_SENSORS {
    double accel[3];    //measured in g
    double temp;        //measured in degs C
    double gyro[3];     //measured in degress per second
    double mag[3];      //measured in uT
};

void sensors_init(void);

void sensors_read_raw(struct RAW_SENSORS *sensors);

/* do count readings and take the median values*/
void sensors_read_uncalibrated(struct COOKED_SENSORS *sensors, int count);

/* do count readings and take the median values and apply the current calibration*/
void sensors_read_cooked(struct COOKED_SENSORS *sensors, int count);

void sensors_uncalibrated_to_cooked(struct COOKED_SENSORS *sensors);

void sensors_get_orientation(gsl_vector *orientation, int count);

/* take a reading */
void sensors_get_reading(void);

/* get the value of the last reading */
gsl_vector* sensors_get_last_reading(void);

#endif
