#ifndef CALIBRATE_H
#define CALIBRATE_H
void quick_cal();

/* this function turns the laser on and off which
 * allows the factory to physically align the 
 * laser pointer with the infra-red range-finder */
void align_cal();

void laser_cal();
void full_cal();
#endif