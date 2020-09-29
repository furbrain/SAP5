#ifndef LASER_H
#define LASER_H
#include <stdbool.h>

enum LASER_SPEED {
    LASER_SLOW,
    LASER_MEDIUM,
    LASER_FAST
    };

/* Turns the laser on or off*/
void laser_on(void);
void laser_off(void);

/* take a reading with the laser and return the uncalibrated distance
   throws ERROR_LASER_READ_FAILED if an invalid reading occurred, or took
   longer than timeout milliseconds*/
double laser_read_raw(enum LASER_SPEED, int timeout);

/* take a reading with the laser and return the calibrated distance
   throws ERROR_LASER_READ_FAILED if an invalid reading occurred, or took
   longer than timeout milliseconds*/
double laser_read(enum LASER_SPEED, int timeout);

#ifdef TEST
void laser_start(enum LASER_SPEED);
bool laser_result_ready(void);
double laser_get_result(void);
#endif
#endif
