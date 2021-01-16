#include <xc.h>
#include <stdbool.h>

#include "version.h"
#include "i2c_util.h"
#include "exception.h"
#include "mem_locations.h"
#include "memory.h"
enum HARDWARE_VERSION version_hardware;

static
const char *adjectives[] = {
    "Angry", 
    "Bored", 
    "Curious", 
    "Devious", 
    "Excited", 
    "Fierce", 
    "Grumpy", 
    "Hungry", 
    "Idle",
    "Jealous"
};

static
const char *animals[] = {
    "Antelope", 
    "Badger", 
    "Cheetah", 
    "Dolphin", 
    "Eagle", 
    "Fox", 
    "Gorilla", 
    "Hamster", 
    "Iguana", 
    "Jaguar"
};

struct SOFTWARE_VERSION version_software PLACE_DATA_AT(VERSION_LOCATION) = {
    .version = {
        .major = VERSION_MAJOR,
        .minor = VERSION_MINOR,
        .revision = VERSION_REVISION,
        .checksum = VERSION_MAJOR ^ VERSION_MINOR ^ VERSION_REVISION ^ 0x55
    },
    .config = {
        .location = APP_CONFIG_LOCATION,
        .size = APP_CONFIG_SIZE,
        .version = VERSION_CONFIG,
    },
    .legs = {
        .location = APP_LEG_LOCATION,
        .size = APP_LEG_SIZE,
        .version = VERSION_LEG,
    },
    .calibration = {
        .location = APP_CALIBRATION_LOCATION,
        .size = APP_CALIBRATION_SIZE,
        .version = VERSION_CALIBRATION,
    }
};

void find_version(void) {
    //look for lsm6ds3 - this implies V1
    if (check_i2c_address(0x6b)) {
        version_hardware = VERSION_V1_1;
        return;
    }
    if (check_i2c_address(0x0e)) {
        version_hardware = VERSION_V1_0;
        return;
    }
    if (check_i2c_address(0x68)) {
        version_hardware = VERSION_ALPHA;
        return;
    }
    version_hardware = VERSION_UNDEFINED;
}

struct VERSION_ID version_get_id() {
    uint8_t *id_byte = (uint8_t*) &UDID1;
    int i;
    int value = 0;
    struct VERSION_ID result;
    for (i=0; i< 20; i++) {
        value ^= *id_byte++;
    }
    value *= 57;
    value %= 100;
    result.adjective = adjectives[value/10];
    result.animal = animals[value%10];
    return result;
}

