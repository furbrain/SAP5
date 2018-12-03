#ifndef LASER_H
#define LASER_H
#include <stdbool.h>

enum LASER_SPEED {
    LASER_SLOW,
    LASER_MEDIUM,
    LASER_FAST
    };

void laser_on(bool enable);
double laser_read(enum LASER_SPEED, int timeout);
void laser_start(enum LASER_SPEED);
bool laser_result_ready(void);
double laser_get_result(void);
#endif
