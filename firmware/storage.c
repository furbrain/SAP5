#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "storage.h"
#include "memory.h"
#include "mem_locations.h"
#include "config.h"
#include "utils.h"
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
    int res;
    while ((*ptr != 0xff) && ptr+sizeof(struct CONFIG) < (config_space+APP_CONFIG_SIZE)) {
        ptr += sizeof(struct CONFIG);
    }
    if (ptr > (config_space + APP_CONFIG_SIZE - sizeof(struct CONFIG))) {
        erase_page((void *)config_space);
        wdt_clear();
        ptr  = config_space;
    }
    res =  write_data((uint8_t *)ptr, (uint8_t *)config, sizeof(struct CONFIG));
    return res;
}


/* if leg spans a page boundary, then return the pointer to the start of the page *
 * oterwise return null */
void* leg_spans_boundary(struct LEG *leg) {
    size_t addr = (size_t)(leg+1);
    if (((addr % 0x800) < sizeof(struct LEG)) && ((addr % 0x800)!=0)){
        return (uint8_t*)(addr - (addr % 0x800));
    } else {    
        return NULL;
    }
}


int write_leg(struct LEG *leg) {
    struct LEG *ptr = (struct LEG*)leg_space;
    void *boundary;
    int res;
    while ((ptr->dt != ULONG_MAX) && (ptr+1 < (struct LEG*)(leg_space+APP_LEG_SIZE))) {
        ptr ++;
    }
    if (ptr+1 > (struct LEG*)(leg_space + APP_LEG_SIZE)) {
        erase_page((void *)leg_space);
        ptr  = (struct LEG*)leg_space;
    } else {
        boundary = leg_spans_boundary(ptr);
        if (boundary) {
            erase_page(boundary);
        }
    }
    wdt_clear();
    res =  write_data((uint8_t *)ptr, (uint8_t *)leg, sizeof(struct LEG));
    return res;
    }

int write_data(void *destination,  const void *source, int length){
    int i, res;
    char *dest = destination;
    const int *src = source;
    for (i=0; i< length; i+=8) {
        res = write_dword(dest, src);
        dest+=8;
        src+=2; 
    }
    return res;
}
