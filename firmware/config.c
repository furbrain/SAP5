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
void config_set_units(int32_t units) {
    config.length_units = (enum LENGTH_UNITS)units;
    config_save();
}


void config_set_style(int32_t style) {
    config.display_style = (enum DISPLAY_STYLE)style;
    config_save();
}


void config_set_day(int32_t on) {
    if (on) {
        day = true;
        display_set_day(day);
    //    laser_set_day(day);
    } else {
        day = false;
        display_set_day(day);
    //    laser_set_day(day);
    }
}
#endif    