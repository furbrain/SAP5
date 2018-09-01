#include <stdint.h>
#include "storage.h"
#include "memory.h"
#include "mem_locations.h"
#include "config.h"
#include <stdio.h>

const uint8_t config_space[APP_CONFIG_SIZE] __attribute__((address(APP_CONFIG_LOCATION), space(prog))) = {[0 ... APP_CONFIG_SIZE-1]=0xff};
const uint8_t leg_space[APP_LEG_SIZE] __attribute__((address(APP_LEG_LOCATION), space(prog))) = {[0 ... APP_LEG_SIZE-1]=0xff};

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
    if (ptr > (config_space + APP_CONFIG_SIZE)) {
        erase_page((void *)config_space);
        ptr  = config_space;
    }
    wdt_clear();
    res =  write_data((uint8_t *)ptr, (uint8_t *)config, sizeof(struct CONFIG));
    return res;
}
