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

enum HARDWARE_VERSION {
    VERSION_ALPHA,
    VERSION_BETA,
    VERSION_V1,
    VERSION_UNDEFINED = 0xff
};    
    
extern enum HARDWARE_VERSION version_hardware;

// Identify the hardware version this code is running on
void find_version(void);


#ifdef	__cplusplus
}
#endif

#endif	/* VERSION_H */

