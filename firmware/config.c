#include "config.h"
#include "i2c_util.h"
#include "display.h"

#ifndef BOOTLOADER
struct CONFIG config = {
            { //calib section
                {{1.0,0,0},{0,1.0,0},{0,0,1.0}}, //accel matrix
                {{1.0,0,0},{0,1.0,0},{0,0,1.0}}, //mag matrix
                1.0                              //laser offset
            },
            POLAR,                               //Polar display style
            METRIC                               //metric units
        };

bool day;

void config_init(){
//    /* check firmware version  and update if appropriate */
//    uint16_t firmware;
//    __read_external(0x0,eeprom,&firmware,2);
//    if (firmware!=FIRMWARE_VERSION) {
//        firmware = FIRMWARE_VERSION;
//        __write_external(0x0,eeprom,&firmware,2);
//    }
//    if (config_store.display_style==0xff) {
//        //initialise config
//        config_save();                   //save defaults to eeprom
//    } else {
//	config = config_store; 		// load saved data from eeprom
//    }
//
}

void config_save(){
//    config_store = config;
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