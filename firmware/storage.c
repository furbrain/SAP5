#include <stdint.h>
#include "storage.h"
#include "memory.h"
#include "mem_locations.h"
#include "config.h"
#include <stdio.h>

#ifndef __DEBUG
const uint8_t config_space[APP_CONFIG_SIZE] __attribute__((address(APP_CONFIG_LOCATION), space(prog))) = {[0 ... APP_CONFIG_SIZE-1]=0xff};
#else
uint8_t config_space[APP_CONFIG_SIZE] __attribute__((address(APP_CONFIG_LOCATION), space(data)))  = {[0 ... APP_CONFIG_SIZE-1]=0xff};
#endif

#ifndef __DEBUG
const uint8_t leg_space[APP_LEG_SIZE] __attribute__((address(APP_LEG_LOCATION), space(prog))) = {[0 ... APP_LEG_SIZE-1]=0xff};
#else
uint8_t  leg_space[APP_LEG_SIZE] __attribute__((address(APP_LEG_LOCATION), space(data)))  = {[0 ... APP_LEG_SIZE-1]=0xff};
#endif

int write_data(uint8_t *dest,  uint8_t *src, int length){
    int i, res;
    char text[22];
    for (i=0; i< length; i+=8) {
        res = write_dword(dest, (int *) src);
        dest+=8;
        src+=8; 
    }
    return res;
}


const struct CONFIG* read_config(void){
    struct CONFIG *config = NULL;
    const uint8_t *ptr = config_space;
    while ((*ptr != 0xff) && ptr < (config_space+APP_CONFIG_SIZE)) {
        config = (struct CONFIG*)ptr;
        ptr += sizeof(struct CONFIG);
    }
    return config;
}

int write_config(struct CONFIG *config) {
    const uint8_t *ptr = config_space;
    char text[24];
    int res;
    while ((*ptr != 0xff) && ptr+sizeof(struct CONFIG) < (config_space+APP_CONFIG_SIZE)) {
        ptr += sizeof(struct CONFIG);
    }
    if (ptr > (config_space + APP_CONFIG_SIZE - sizeof(struct CONFIG))) {
        erase_page((void *)config_space);
        ptr  = config_space;
    }
    wdt_clear();
    res =  write_data((uint8_t *)ptr, (uint8_t *)config, sizeof(struct CONFIG));
    return res;
}


int write_leg(struct LEG *leg) {
    const uint8_t *ptr = leg_space;
    char text[24];
    int res;
    while ((*ptr != 0xff) && ptr+sizeof(struct LEG) < (leg_space+APP_LEG_SIZE)) {
        ptr += sizeof(struct LEG);
    }
    if (ptr > (leg_space + APP_LEG_SIZE - sizeof(struct LEG))) {
        erase_page((void *)leg_space);
        ptr  = leg_space;
    }
    wdt_clear();
    res =  write_data((uint8_t *)ptr, (uint8_t *)leg, sizeof(struct LEG));
    return res;
    }
