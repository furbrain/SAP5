#include <stdbool.h>

#include "version.h"
#include "i2c_util.h"
#include "exception.h"
 enum HARDWARE_VERSION version_hardware;

void find_version(void) {
    //look for lsm6ds3 - this implies V1
    if (check_i2c_address(0x6b)) {
        version_hardware = VERSION_V1;
        return;
    }
    if (check_i2c_address(0x0e)) {
        version_hardware = VERSION_BETA;
        return;
    }
    if (check_i2c_address(0x68)) {
        version_hardware = VERSION_ALPHA;
        return;
    }
    version_hardware = VERSION_UNDEFINED;
}

