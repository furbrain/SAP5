/* 
 * File:   app_type.h
 * Author: phil
 *
 * Created on 04 August 2018, 11:02
 */

#ifndef APP_TYPE_H
#define	APP_TYPE_H

/*define one of BOOTLOADER, BOOTLOADER_APP or STANDALONE*/
#if defined (STANDALONE)
#else
#define BOOTLOADER_APP
#endif

#endif	/* APP_TYPE_H */