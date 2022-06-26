/* 
 * File:   version.h
 * Author: phil
 *
 * Created on 09 March 2022, 12:25
 */

#ifndef BL_VERSION_H
#define	BL_VERSION_H

#ifdef	__cplusplus
extern "C" {
#endif
#define BL_VERSION_MAJOR 1
#define BL_VERSION_MINOR 1
#ifdef NEW_CASE
#define CASE_LENGTH 133
#else    
#define CASE_LENGTH 90
#endif

    
struct BL_VERSION {
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t case_length;
        uint8_t checksum;
    } version;
};

extern const struct BL_VERSION version_bootloader;


#ifdef	__cplusplus
}
#endif

#endif	/* VERSION_H */

