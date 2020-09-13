/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : chat.c
* Author             : AMS - VMA RF  Application team
* Version            : V1.0.0
* Date               : 01-December-2015
* Description        : This file handles bytes received from USB and the 
*                      chat device configuration, connection..... 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Include -------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "gp_timer.h"
#include "ble_const.h" 
#include "app_state.h"
#include "bluenrg1_stack.h"
#include "osal.h"
#include "gatt_db.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* discovery procedure mode context */
typedef struct discoveryContext_s {
  uint8_t check_disc_proc_timer;
  uint8_t check_disc_mode_timer;
  uint8_t is_device_found; 
  uint8_t do_connect;
  tClockTime startTime;
  uint8_t device_found_address_type;
  uint8_t device_found_address[6];
  uint16_t device_state;
} discoveryContext_t;

/* Private defines -----------------------------------------------------------*/
#define BLE_NEW_CHAT_COMPLETE_LOCAL_NAME_SIZE 16 
#define CMD_BUFF_SIZE 512

#define DISCOVERY_TIMEOUT 3000 /* at least 3 seconds */

#define DISCOVERY_PROC_SCAN_INT 0x4000 
#define DISCOVERY_PROC_SCAN_WIN 0x4000
#define ADV_INT_MIN 0x90
#define ADV_INT_MAX 0x90 

/* Private macros ------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 1
#endif
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Private variables ---------------------------------------------------------*/
static discoveryContext_t discovery;
volatile int app_flags = SET_CONNECTABLE;
volatile uint16_t connection_handle = 0;
extern uint16_t chatServHandle, TXCharHandle, RXCharHandle; 

/* UUIDs */
UUID_t UUID_Tx;
UUID_t UUID_Rx;

uint16_t tx_handle, rx_handle; 
uint16_t discovery_time = 0; 
uint8_t device_role = 0xFF;
uint8_t counter = 0;
uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G','_','N','e','w','C','h','a','t'}; 

static char cmd[CMD_BUFF_SIZE];
static uint16_t cmd_buff_end = 0, cmd_buff_start = 0;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Process_InputData.
* Description    : Process a command. It should be called when data are received.
* Input          : data_buffer: data address.
*	           Nb_bytes: number of received bytes.
* Return         : none.
*******************************************************************************/
void Process_InputData(uint8_t* data_buffer, uint16_t Nb_bytes)
{
  uint8_t i;
  
  for (i = 0; i < Nb_bytes; i++) {
    if(cmd_buff_end >= CMD_BUFF_SIZE-1)
      cmd_buff_end = 0;
    
    cmd[cmd_buff_end] = data_buffer[i];
    cmd_buff_end++;
    
    if(cmd[cmd_buff_end-1] == '\n') {
      if(cmd_buff_end != 1) {
        
        cmd[cmd_buff_end] = '\0'; // Only a termination character. Not strictly needed.
        
        // Set flag to send data. Disable UART IRQ to avoid overwriting buffer with new incoming data
        APP_FLAG_SET(SEND_DATA);
        NVIC_DisableIRQ(UART_IRQn);
        
        cmd_buff_start = 0; 
      }
      else {
        cmd_buff_end = 0; // Discard
      }      
    }
  }
}

/*******************************************************************************
* Function Name  : Reset_DiscoveryContext.
* Description    : Reset the discovery context.
* Input          : None.
* Return         : None.
*******************************************************************************/
void Reset_DiscoveryContext(void)
{
  discovery.check_disc_proc_timer = FALSE;
  discovery.check_disc_mode_timer = FALSE;
  discovery.is_device_found = FALSE; 
  discovery.do_connect = FALSE;
  discovery.startTime = 0;
  discovery.device_state = INIT;
  Osal_MemSet(&discovery.device_found_address[0], 0, 6);
  device_role = 0xFF; 
}

/*******************************************************************************
* Function Name  : Setup_DeviceAddress.
* Description    : Setup the device address.
* Input          : None.
* Return         : None.
*******************************************************************************/
void Setup_DeviceAddress(void)
{
  tBleStatus ret;
  uint8_t bdaddr[] = {0x00, 0x00, 0x00, 0xE1, 0x80, 0x02};
  uint8_t random_number[8];
  
  /* get a random number from BlueNRG */ 
  ret = hci_le_rand(random_number);
  if(ret != BLE_STATUS_SUCCESS)
     PRINTF("hci_le_rand() call failed: 0x%02x\n", ret);
  
  
  discovery_time = DISCOVERY_TIMEOUT; 

  /* setup discovery time with random number */
  for (uint8_t i=0; i<8; i++) {
    discovery_time += (2*random_number[i]);
  } 
  
  /* Setup last 3 bytes of public address with random number */
  bdaddr[0] = (uint8_t) (random_number[0]); 
  bdaddr[1] = (uint8_t) (random_number[3]); 
  bdaddr[2] = (uint8_t) (random_number[6]);
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);
  if(ret != BLE_STATUS_SUCCESS) {
      PRINTF("Setting BD_ADDR failed 0x%02x\n", ret);
  } else {
    PRINTF("Public address: ");
    for (uint8_t i=5; i>0; i--) {
      PRINTF("%02X-", bdaddr[i]);
    }
    PRINTF("%02X\n", bdaddr[0]);
  }
}

/*******************************************************************************
* Function Name  : Find_DeviceName.
* Description    : Extracts the device name.
* Input          : Data length.
*                  Data value
* Return         : TRUE if the local name found is the expected one, FALSE otherwise.
*******************************************************************************/
uint8_t Find_DeviceName(uint8_t data_length, uint8_t *data_value)
{
  uint8_t index = 0;
  
  while (index < data_length) {
    /* Advertising data fields: len, type, values */
    /* Check if field is complete local name and the lenght is the expected one for BLE NEW Chat  */
    if (data_value[index+1] == AD_TYPE_COMPLETE_LOCAL_NAME) {
      /* check if found device name is the expected one: local_name */ 
      if (memcmp(&data_value[index+1], &local_name[0], BLE_NEW_CHAT_COMPLETE_LOCAL_NAME_SIZE) == 0)
        return TRUE;
      else
        return FALSE;
    } else {
      /* move to next advertising field */
      index += (data_value[index] +1); 
    }
  }
  
  return FALSE;
}

/*******************************************************************************
* Function Name  : CHAT_DeviceInit.
* Description    : Init Chat device.
* Input          : None.
* Return         : Status.
*******************************************************************************/
uint8_t CHAT_DeviceInit(void)
{
  uint8_t ret;
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  /* Setup the device address */
  Setup_DeviceAddress();

  /* Set the TX power to -2 dBm */
  aci_hal_set_tx_power_level(1, 4);

  /* GATT Init */
  ret = aci_gatt_init();    
  if(ret != BLE_STATUS_SUCCESS) {
      PRINTF("GATT_Init failed: 0x%02x\n", ret);
      return ret;
  }

  /* GAP Init */
  ret = aci_gap_init(GAP_CENTRAL_ROLE|GAP_PERIPHERAL_ROLE, 0,0x07, &service_handle, 
                     &dev_name_char_handle, &appearance_char_handle);
  if(ret != BLE_STATUS_SUCCESS) {
    PRINTF("GAP_Init failed: 0x%02x\n", ret);
    return ret;
  }

  /* Add Device Service & Characteristics */
  ret = Add_ChatService();
  if(ret != BLE_STATUS_SUCCESS) {
    PRINTF("Error while adding service: 0x%02x\n", ret);
    return ret;
  }

  /* Reset the discovery context */
  Reset_DiscoveryContext();

  return BLE_STATUS_SUCCESS;
}

void Send_Data_Over_BLE(void)
{
  if(!APP_FLAG(SEND_DATA) || APP_FLAG(TX_BUFFER_FULL))
    return;
  
  while(cmd_buff_start < cmd_buff_end){
    uint32_t len = MIN(20, cmd_buff_end - cmd_buff_start);
    
    if(device_role == SLAVE_ROLE) {
      if(aci_gatt_update_char_value_ext(connection_handle,chatServHandle,TXCharHandle,1,len,0, len,(uint8_t *)cmd+cmd_buff_start)==BLE_STATUS_INSUFFICIENT_RESOURCES){ 
        APP_FLAG_SET(TX_BUFFER_FULL);
        return;
      }
    }else if(device_role == MASTER_ROLE) {
      if(aci_gatt_write_without_resp(connection_handle, rx_handle+1, len, (uint8_t *)cmd+cmd_buff_start)==BLE_STATUS_INSUFFICIENT_RESOURCES){
        APP_FLAG_SET(TX_BUFFER_FULL);
        return;
      }
    }
    cmd_buff_start += len;
  }
  
  // All data from buffer have been sent.
  APP_FLAG_CLEAR(SEND_DATA);
  cmd_buff_end = 0;
  NVIC_EnableIRQ(UART_IRQn);
}

/*******************************************************************************
* Function Name  : Connection_StateMachine.
* Description    : Connection state machine.
* Input          : None.
* Return         : None.
*******************************************************************************/
void Connection_StateMachine(void)
{  
  uint8_t ret;
   
  switch (discovery.device_state) {
  case (INIT):
    {
      Reset_DiscoveryContext();
      discovery.device_state = START_DISCOVERY_PROC;
    }
    break; /* end case (INIT) */
  case (START_DISCOVERY_PROC):
    {
      ret = aci_gap_start_general_discovery_proc(DISCOVERY_PROC_SCAN_INT, DISCOVERY_PROC_SCAN_WIN, PUBLIC_ADDR, 0x00); 
      if (ret != BLE_STATUS_SUCCESS) {
        PRINTF("aci_gap_start_general_discovery_proc() failed: %02X\n",ret);
        discovery.device_state = DISCOVERY_ERROR; 
      } else {
        PRINTF("aci_gap_start_general_discovery_proc OK\n");  
        discovery.startTime = Clock_Time();
        discovery.check_disc_proc_timer = TRUE; 
        discovery.check_disc_mode_timer = FALSE; 
        discovery.device_state = WAIT_TIMER_EXPIRED; 
      }
    }
    break;/* end case (START_DISCOVERY_PROC) */
  case (WAIT_TIMER_EXPIRED):
    {
      /* Verify if startTime check has to be done  since discovery procedure is ongoing */
      if (discovery.check_disc_proc_timer == TRUE) {
        /* check startTime value */
        if (Clock_Time() - discovery.startTime > discovery_time) {  
          discovery.check_disc_proc_timer = FALSE; 
          discovery.startTime = 0;
          discovery.device_state = DO_TERMINATE_GAP_PROC; 
          
        }/* if (Clock_Time() - discovery.startTime > discovery_time) */
      }  
      /* Verify if startTime check has to be done  since discovery mode is ongoing */
      else if (discovery.check_disc_mode_timer == TRUE) {
        /* check startTime value */
        if (Clock_Time() - discovery.startTime > discovery_time) {  
          discovery.check_disc_mode_timer = FALSE; 
          discovery.startTime = 0;
          
          /* Discovery mode is ongoing: set non discoverable mode */
          discovery.device_state = DO_NON_DISCOVERABLE_MODE; 
          
        }/* else if (discovery.check_disc_mode_timer == TRUE) */
      }/* if ((discovery.check_disc_proc_timer == TRUE) */
    }
    break; /* end case (WAIT_TIMER_EXPIRED) */
  case (DO_NON_DISCOVERABLE_MODE):
    {
      ret = aci_gap_set_non_discoverable();
      if (ret != BLE_STATUS_SUCCESS) {
        PRINTF("aci_gap_set_non_discoverable() failed: 0x%02x\n", ret);
        discovery.device_state = DISCOVERY_ERROR; 
      } else {
        PRINTF("aci_gap_set_non_discoverable() OK\n"); 
        /* Restart Central discovery procedure */
        discovery.device_state = INIT; 
      }
    }
    break; /* end case (DO_NON_DISCOVERABLE_MODE) */
  case (DO_TERMINATE_GAP_PROC):
    {
      /* terminate gap procedure */
      ret = aci_gap_terminate_gap_proc(0x02); // GENERAL_DISCOVERY_PROCEDURE
      if (ret != BLE_STATUS_SUCCESS) 
      {
        PRINTF("aci_gap_terminate_gap_procedure() failed: 0x%02x\n", ret);
        discovery.device_state = DISCOVERY_ERROR; 
        break;
      }
      else 
      {
        PRINTF("aci_gap_terminate_gap_procedure() OK\n");
        discovery.device_state = WAIT_EVENT; /* wait for GAP procedure complete */
      }
    }
    break; /* end case (DO_TERMINATE_GAP_PROC) */
  case (DO_DIRECT_CONNECTION_PROC):
    {
      PRINTF("Device Found with address: ");
      for (uint8_t i=5; i>0; i--) {
        PRINTF("%02X-", discovery.device_found_address[i]);
      }
      PRINTF("%02X\n", discovery.device_found_address[0]);
      /* Do connection with first discovered device */ 
      ret = aci_gap_create_connection(DISCOVERY_PROC_SCAN_INT, DISCOVERY_PROC_SCAN_WIN,
                                      discovery.device_found_address_type, discovery.device_found_address,
                                      PUBLIC_ADDR, 40, 40, 0, 60, 2000 , 2000); 
      if (ret != BLE_STATUS_SUCCESS) {
        PRINTF("aci_gap_create_connection() failed: 0x%02x\n", ret);
        discovery.device_state = DISCOVERY_ERROR; 
      } else {
        PRINTF("aci_gap_create_connection() OK\n");
        discovery.device_state = WAIT_EVENT;
      }
    }
    break; /* end case (DO_DIRECT_CONNECTION_PROC) */
  case (WAIT_EVENT):
    {
      discovery.device_state = WAIT_EVENT;
    }
    break; /* end case (WAIT_EVENT) */
  case (ENTER_DISCOVERY_MODE):
    {
      /* Put Peripheral device in discoverable mode */
      
      /* disable scan response */
      hci_le_set_scan_response_data(0,NULL);
      
      ret = aci_gap_set_discoverable(ADV_IND, ADV_INT_MIN, ADV_INT_MAX, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                     sizeof(local_name), local_name, 0, NULL, 0x6, 0x8); 
      if (ret != BLE_STATUS_SUCCESS) {
        PRINTF("aci_gap_set_discoverable() failed: 0x%02x\n", ret);
        discovery.device_state = DISCOVERY_ERROR; 
      } else {
        PRINTF("aci_gap_set_discoverable() OK\n"); 
        discovery.startTime = Clock_Time();
        discovery.check_disc_mode_timer = TRUE; 
        discovery.check_disc_proc_timer = FALSE;
        discovery.device_state = WAIT_TIMER_EXPIRED; 
      }
    }
    break; /* end case (ENTER_DISCOVERY_MODE) */    
  case (DISCOVERY_ERROR):
    {
      Reset_DiscoveryContext();
    }
    break; /* end case (DISCOVERY_ERROR) */
  default:
    break;
   }/* end switch */

}/* end Connection_StateMachine() */

/*******************************************************************************
* Function Name  : APP_Tick.
* Description    : Application tick to run the state machine.
* Input          : None.
* Return         : None.
*******************************************************************************/
void APP_Tick(void)
{
  if(APP_FLAG(SET_CONNECTABLE)) {
    Connection_StateMachine();
  }
  
  Send_Data_Over_BLE();
        
  if (device_role == MASTER_ROLE) {
    /* Start TX handle Characteristic discovery if not yet done */
    if (APP_FLAG(CONNECTED) && !APP_FLAG(END_READ_TX_CHAR_HANDLE)) {
      if (!APP_FLAG(START_READ_TX_CHAR_HANDLE)) {
        /* Discovery TX characteristic handle by UUID 128 bits */
        const uint8_t charUuid128_TX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
        
        Osal_MemCpy(&UUID_Tx.UUID_16, charUuid128_TX, 16);
        aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF,UUID_TYPE_128,&UUID_Tx);
        APP_FLAG_SET(START_READ_TX_CHAR_HANDLE);
      }
    }
    /* Start RX handle Characteristic discovery if not yet done */
    else if (APP_FLAG(CONNECTED) && !APP_FLAG(END_READ_RX_CHAR_HANDLE)) {
      /* Discovery RX characteristic handle by UUID 128 bits */
      if (!APP_FLAG(START_READ_RX_CHAR_HANDLE)) {
        /* Discovery RX characteristic handle by UUID 128 bits */
        const uint8_t charUuid128_RX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};
        
        Osal_MemCpy(&UUID_Rx.UUID_16, charUuid128_RX, 16);
        aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF,UUID_TYPE_128,&UUID_Rx);
        APP_FLAG_SET(START_READ_RX_CHAR_HANDLE);
      }
    }
      
    if(APP_FLAG(CONNECTED) && APP_FLAG(END_READ_TX_CHAR_HANDLE) && APP_FLAG(END_READ_RX_CHAR_HANDLE) && !APP_FLAG(NOTIFICATIONS_ENABLED)) {
      uint8_t client_char_conf_data[] = {0x01, 0x00}; // Enable notifications
      struct timer t;
      Timer_Set(&t, CLOCK_SECOND*10);
          
      while(aci_gatt_write_char_desc(connection_handle, tx_handle+2, 2, client_char_conf_data)==BLE_STATUS_NOT_ALLOWED) { 
        // Radio is busy.
        if(Timer_Expired(&t)) break;
      }
      APP_FLAG_SET(NOTIFICATIONS_ENABLED);
    }
  }/* if (device_role == MASTER_ROLE) */      
}


/* ***************** BlueNRG-1 Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : aci_gap_proc_complete_event.
 * Description    : This event indicates the end of a GAP procedure.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gap_proc_complete_event(uint8_t Procedure_Code,
                                 uint8_t Status,
                                 uint8_t Data_Length,
                                 uint8_t Data[])
{
  if (Procedure_Code == GAP_GENERAL_DISCOVERY_PROC) { 
    /* gap procedure complete has been raised as consequence of a GAP 
       terminate procedure done after a device found event during the discovery procedure */
    if (discovery.do_connect == TRUE) {
      discovery.do_connect = FALSE;
      discovery.check_disc_proc_timer = FALSE;
      discovery.startTime = 0; 
      /* discovery procedure has been completed and device found:
         go to connection mode */
      discovery.device_state = DO_DIRECT_CONNECTION_PROC;
    } else {
      /* discovery procedure has been completed and no device found:
         go to discovery mode */
      discovery.check_disc_proc_timer = FALSE;
      discovery.startTime = 0; 
      discovery.device_state = ENTER_DISCOVERY_MODE;
    }
  }
}

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates the end of a connection procedure.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)

{ 
  /* Set the exit state for the Connection state machine: APP_FLAG_CLEAR(SET_CONNECTABLE); */
  APP_FLAG_CLEAR(SET_CONNECTABLE); 
  discovery.check_disc_proc_timer = FALSE;
  discovery.check_disc_mode_timer = FALSE;
  discovery.startTime = 0; 
  
  connection_handle = Connection_Handle;
  
  APP_FLAG_SET(CONNECTED); 
  discovery.device_state = INIT;
  
  /* store device role */
  device_role = Role;
  
}/* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event indicates the discconnection from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
  APP_FLAG_CLEAR(CONNECTED);
  /* Make the device connectable again. */
  APP_FLAG_SET(SET_CONNECTABLE);
  APP_FLAG_CLEAR(NOTIFICATIONS_ENABLED);
  
  APP_FLAG_CLEAR(START_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(END_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(START_READ_RX_CHAR_HANDLE); 
  APP_FLAG_CLEAR(END_READ_RX_CHAR_HANDLE); 
  APP_FLAG_CLEAR(TX_BUFFER_FULL);
  
  Reset_DiscoveryContext();

}/* end hci_disconnection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_le_advertising_report_event.
 * Description    : An advertising report is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_advertising_report_event(uint8_t Num_Reports,
                                     Advertising_Report_t Advertising_Report[])
{
  /* Advertising_Report contains all the expected parameters */
  uint8_t evt_type = Advertising_Report[0].Event_Type ;
  uint8_t data_length = Advertising_Report[0].Length_Data;
  uint8_t bdaddr_type = Advertising_Report[0].Address_Type;
  uint8_t bdaddr[6];

  Osal_MemCpy(bdaddr, Advertising_Report[0].Address,6);
      
  /* BLE New Chat device not yet found: check current device found */
  if (!(discovery.is_device_found)) { 
    /* BLE New Chat device not yet found: check current device found */
    if ((evt_type == ADV_IND) && Find_DeviceName(data_length, Advertising_Report[0].Data)) {
      discovery.is_device_found = TRUE; 
      discovery.do_connect = TRUE;  
      discovery.check_disc_proc_timer = FALSE;
      discovery.check_disc_mode_timer = FALSE;
      /* store first device found:  address type and address value */
      discovery.device_found_address_type = bdaddr_type;
      Osal_MemCpy(discovery.device_found_address, bdaddr, 6);
      /* device is found: terminate discovery procedure */
      discovery.device_state = DO_TERMINATE_GAP_PROC;      
    }
  }  
} /* hci_le_advertising_report_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : Attribute modified from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
  Attribute_Modified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);      
} /* end aci_gatt_attribute_modified_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_notification_event.
 * Description    : Notification received from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[])
{ 
  if(Attribute_Handle == tx_handle+1) {
    for(volatile uint8_t i = 0; i < Attribute_Value_Length; i++)
    	printf("%c", Attribute_Value[i]);
  }
} /* end aci_gatt_notification_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_disc_read_char_by_uuid_resp_event.
 * Description    : Read characteristic by UUID from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle,
                                                uint16_t Attribute_Handle,
                                                uint8_t Attribute_Value_Length,
                                                uint8_t Attribute_Value[])
{
  printf("aci_gatt_disc_read_char_by_uuid_resp_event, Connection Handle: 0x%04X\n", Connection_Handle);
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE)) {
    tx_handle = Attribute_Handle; 
    printf("TX Char Handle 0x%04X\n", tx_handle);
  } else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE)) {
    rx_handle = Attribute_Handle;
    printf("RX Char Handle 0x%04X\n", rx_handle);
  }
} /* end aci_gatt_disc_read_char_by_uuid_resp_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_proc_complete_event.
 * Description    : GATT procedure complet event.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle,
                                  uint8_t Error_Code)
{ 
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE)) {
    APP_FLAG_SET(END_READ_TX_CHAR_HANDLE);
  } else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE)) {
    APP_FLAG_SET(END_READ_RX_CHAR_HANDLE);
  }
} /* end aci_gatt_proc_complete_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_tx_pool_available_event.
 * Description    : GATT TX pool available event.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{      
  APP_FLAG_CLEAR(TX_BUFFER_FULL);
} /* end aci_gatt_tx_pool_available_event() */
