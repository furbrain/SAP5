#include <stdint.h>
#include "storage.h"
#include "memory.h"
#include "mem_locations.h"
#include "config.h"

CONST_STORE uint8_t config_space[APP_CONFIG_SIZE] PLACE_DATA_AT(APP_CONFIG_LOCATION) = {[0 ... APP_CONFIG_SIZE-1]=0xff};

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
        ptr  = config_space;
    }
    res =  write_data((uint8_t *)ptr, (uint8_t *)config, sizeof(struct CONFIG));
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
