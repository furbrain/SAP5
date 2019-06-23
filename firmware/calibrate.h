#ifndef CALIBRATE_H
#define CALIBRATE_H

#define CALIBRATION_SUB_SAMPLES 50
#define CALIBRATION_POSITIONS_PER_AXIS 8
#define CALIBRATION_SAMPLES (CALIBRATION_POSITIONS_PER_AXIS * 2)

void calibrate_axes(int32_t dummy);
void calibrate_laser(int32_t dummy);
void calibrate_sensors(int32_t dummy);
#endif
