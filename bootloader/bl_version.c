#include <stdint.h>

#include "mem_locations.h"
#include "memory.h"
#include "bl_version.h"

const struct BL_VERSION version_bootloader PLACE_DATA_AT(BL_VERSION_LOCATION) = {
    .version = {
        .major = BL_VERSION_MAJOR,
        .minor = BL_VERSION_MINOR,
        .case_length = CASE_LENGTH,
        .checksum = BL_VERSION_MAJOR ^ BL_VERSION_MINOR ^ CASE_LENGTH ^ 0x55
    }
};
