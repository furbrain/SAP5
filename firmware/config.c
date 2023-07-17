#include <stdio.h>

#include "display.h"
#include "memory.h"
#include "mem_locations.h"
#include "exception.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "config.h"
#include "version.h"

union CONFIG_STORE config_store PLACE_DATA_AT(APP_CONFIG_LOCATION) = {.raw = {[0 ... APP_CONFIG_SIZE-1]=0xff}};

const 
struct CONFIG default_config = {
    {{9,9,9}, {9,9,9}}, //use default axes for hardware
        { //calib section
            {1,0,0, 0,1,0, 0,0,1, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, //accel matrix
            {1,0,0, 0,1,0, 0,0,1, 0,0,0, 0,0,0, 0,0,0, 0,0,0}, //mag matrix
            0.090                              //laser offset
        },
        POLAR,                               //Polar display style
        METRIC,                              //metric units
        120,                                  //2 minute timeout
        false,								 //no bluetooth module
        false,                                //normal display style (not compact)
        false                                //don't discard measurements by default
    };

struct CONFIG config;

void config_save(void){
    CONST_STORE struct CONFIG *ptr = config_store.configs;
    CONST_STORE struct CONFIG *overflow = &config_store.configs[MAX_CONFIG_COUNT];
    while ((ptr < overflow) && (ptr->axes.accel[0] != 0xff)) {
        ptr ++;
    }
    if (ptr >= overflow) {
        memory_erase_page(config_store.raw);
        ptr  = &config_store.configs[0];
    }
    memory_write_data(ptr, &config, sizeof(config));
}

bool config_ptr_is_valid(const struct CONFIG *conf) {
    return conf->axes.accel[0] != 0xff;
}

void config_load(void){
    CONST_STORE struct CONFIG *ptr = &config_store.configs[0];
    CONST_STORE struct CONFIG *overflow = &config_store.configs[MAX_CONFIG_COUNT];
    config = default_config;
    config.calib.laser_offset = version_get_case_length()/1000.0;
    while (config_ptr_is_valid(ptr) && ptr < overflow) {
        config = *ptr;
        ptr++;
    }
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


void config_set_compact(int32_t on) {
    config.compact = on;
    config_save();
}

void config_set_timeout(int32_t timeout) {
    config.timeout = timeout;
    config_save();
}

void config_set_discard(int32_t on) {
    config.discard = on;
    config_save();
}
