#ifndef CALIBRATE_H
#define CALIBRATE_H

#define CAL_AXIS_COUNT 4
#define CAL_TARGET_COUNT 8
#define CALIBRATION_SAMPLES (CAL_AXIS_COUNT*2 + CAL_TARGET_COUNT*2)
#define MAG_DEGS_PER_SAMPLE 10
#define MAG_SAMPLES_PER_AXIS (360/MAG_DEGS_PER_SAMPLE)
#define MAG_EXTRA_SAMPLES (MAG_SAMPLES_PER_AXIS * 3)
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include "maths.h"
extern struct menu calibration_menu;

double check_accuracy(const gsl_matrix *mag, const calibration *mag_cal,
                      const gsl_matrix *grav, const calibration *grav_cal);
double check_accuracy2(const gsl_matrix *mag, const calibration *mag_cal,
                      const gsl_matrix *grav, const calibration *grav_cal);
double check_calibration(const gsl_matrix *data, calibration *cal);

void calibrate_axes(int32_t dummy);
void calibrate_laser(int32_t dummy);
void calibrate_sensors(int32_t dummy);
#endif
