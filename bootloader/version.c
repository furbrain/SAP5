#include <stdint.h>

#include "mem_locations.h"
#include "memory.h"
#include "version.h"

const struct BL_VERSION version_bootloader PLACE_DATA_AT(BL_VERSION_LOCATION) = {
    .version = {
        .major = BL_VERSION_MAJOR,
        .minor = BL_VERSION_MINOR,
        .revision = BL_VERSION_REVISION,
        .checksum = BL_VERSION_MAJOR ^ BL_VERSION_MINOR ^ BL_VERSION_REVISION ^ 0x55
    }
};
