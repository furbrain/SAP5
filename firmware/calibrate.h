#ifndef CALIBRATE_H
#define CALIBRATE_H

#define CAL_AXIS_COUNT 4
#define CAL_TARGET_COUNT 8
#define CALIBRATION_SAMPLES (CAL_AXIS_COUNT*2 + CAL_TARGET_COUNT)
extern struct menu calibration_menu;

void calibrate_axes(int32_t dummy);
void calibrate_laser(int32_t dummy);
void calibrate_sensors(int32_t dummy);
#endif
