#include <stdio.h>
#include "config.h"
#include "i2c_util.h"
#include "display.h"
#include "storage.h"

#ifndef BOOTLOADER
struct CONFIG config = {
            {{1,0,2},{0,1,5}},                   //axis orientation
            { //calib section
                {{1.0k,0k,0k},{0k,1.0k,0k},{0k,0k,1.0k}}, //accel matrix
                {{1.0k,0k,0k},{0k,1.0k,0k},{0k,0k,1.0k}}, //mag matrix
                0.090k                              //laser offset
            },
            POLAR,                               //Polar display style
            METRIC                               //metric units
        };

bool day;

void config_init(){
    const struct CONFIG* temp_config;
    temp_config = read_config();
    if (temp_config != NULL) {
        config  = *temp_config;
    }
}

void config_save(){
    write_config(&config);
}

/* config management */
void config_set_metric() {
    config.length_units = METRIC;
    config_save();
}

void config_set_imperial() {
    config.length_units = IMPERIAL;
    config_save();
}

void config_set_cartesian() {
    config.display_style = CARTESIAN;
    config_save();
}

void config_set_polar() {
    config.display_style = POLAR;
    config_save();
}

void config_set_grad() {
    config.display_style = GRAD;
    config_save();
}

void set_day() {
    day = true;
    display_set_day(day);
//    laser_set_day(day);
}

void set_night() {
    day = false;
    display_set_day(day);
//    laser_set_day(day);
}
#endif    