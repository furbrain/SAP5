/* 
 * File:   usb_callbacks.h
 * Author: phil
 *
 * Created on August 5, 2018, 8:33 PM
 */

#ifndef USB_CALLBACKS_H
#define	USB_CALLBACKS_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "mcc_generated_files/usb/usb_ch9.h"

int8_t app_unknown_setup_request_callback();
void reset_cb(void);

#ifdef	__cplusplus
}
#endif

#endif	/* USB_CALLBACKS_H */

