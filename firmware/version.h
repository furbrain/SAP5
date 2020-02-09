/* 
 * File:   version.h
 * Author: phil
 *
 * Created on 29 January 2020, 20:44
 */

#ifndef VERSION_H
#define	VERSION_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
//Version 1.1.0
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_REVISION 0

#define VERSION_CONFIG 1
#define VERSION_LEG 1
#define VERSION_CALIBRATION 1

enum HARDWARE_VERSION {
    VERSION_ALPHA,
    VERSION_V1_0,
    VERSION_V1_1,
    VERSION_UNDEFINED = 0xff
};    
    
extern enum HARDWARE_VERSION version_hardware;

struct SOFTWARE_VERSION {
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t revision;
        uint8_t checksum;
    } version;
    struct {
        uint32_t location;
        uint16_t size;
        uint8_t version;
    } config;
    struct {
        uint32_t location;
        uint16_t size;
        uint8_t version;
    } legs;
    struct {
        uint32_t location;
        uint16_t size;
        uint8_t version;
    } calibration;
};

extern struct SOFTWARE_VERSION version_software;

// Identify the hardware version this code is running on
void find_version(void);


#ifdef	__cplusplus
}
#endif

#endif	/* VERSION_H */

