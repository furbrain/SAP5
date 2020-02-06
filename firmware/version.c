#include <stdbool.h>

#include "version.h"
#include "i2c_util.h"
#include "exception.h"
#include "mem_locations.h"
#include "memory.h"
enum HARDWARE_VERSION version_hardware;

struct SOFTWARE_VERSION version_software PLACE_DATA_AT(VERSION_LOCATION) = {
    .version = {
        .major = VERSION_MAJOR,
        .minor = VERSION_MINOR,
        .revision = VERSION_REVISION,
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

