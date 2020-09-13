/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : app_state.h
* Author             : AMS - VMA, RF Application Team
* Version            : V1.0.0
* Date               : 21-Sept-2015
* Description        : Header file wich contains variable used for application.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Exported variables ------------------------------------------------------- */  
/** 
  * @brief  Device Role
  */      
#define MASTER_ROLE 0x00
#define SLAVE_ROLE 0x01
 
/** 
  * @brief  Discovery States
 */  
#define INIT                      0x00
#define START_DISCOVERY_PROC      0x01
#define WAIT_EVENT                0x02
#define WAIT_TIMER_EXPIRED        0x04
#define DO_DIRECT_CONNECTION_PROC 0x08
#define ENTER_DISCOVERY_MODE      0x10
#define DO_TERMINATE_GAP_PROC     0x20
#define DO_NON_DISCOVERABLE_MODE  0x40
#define DISCOVERY_ERROR           0x80
 
/** 
  * @brief  Variable which contains some flags useful for application
  */ 
extern volatile int app_flags;

/** 
  * @brief  Flags for application
  */ 
#define SET_CONNECTABLE           0x000100
#define CONNECTED                 0x000200
#define NOTIFICATIONS_ENABLED     0x000400

/* Added flags for handling TX, RX characteristics discovery */
#define START_READ_TX_CHAR_HANDLE 0x000800 
#define END_READ_TX_CHAR_HANDLE   0x001000
#define START_READ_RX_CHAR_HANDLE 0x002000
#define END_READ_RX_CHAR_HANDLE   0x004000

/* GATT EVT_BLUE_GATT_TX_POOL_AVAILABLE event */
#define TX_BUFFER_FULL            0x008000
#define SEND_DATA                 0x010000

/* Exported macros -----------------------------------------------------------*/
#define APP_FLAG(flag) (app_flags & flag)

#define APP_FLAG_SET(flag) (app_flags |= flag)
#define APP_FLAG_CLEAR(flag) (app_flags &= ~flag)


#ifdef __cplusplus
}
#endif

#endif /*__APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
