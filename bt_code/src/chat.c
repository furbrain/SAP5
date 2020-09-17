/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : chat.c
* Author             : AMS - VMA RF  Application team
* Version            : V1.0.0
* Date               : 30-November-2015
* Description        : This file handles bytes received from USB and the init
*                      function. 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "gp_timer.h"
#include "ble_const.h" 
#include "bluenrg1_stack.h"
#include "bluenrg1_api.h"
#include "app_state.h"
#include "osal.h"
#include "gatt_db.h"
#include "chat.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#define CMD_BUFF_SIZE 512

#if SERVER
  #define SERVER_ADDRESS 0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02
  #define LOCAL_NAME  'C','h','a','t','_','1','_','2'
  #define MANUF_DATA_SIZE 27
#endif 

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint8_t connInfo[20];
volatile int app_flags = SET_CONNECTABLE;
volatile uint16_t connection_handle = 0;

/** 
  * @brief  Handle of TX,RX  Characteristics.
  */ 
#ifdef CLIENT
uint16_t tx_handle;
uint16_t rx_handle;
#endif 

/* UUIDs */
UUID_t UUID_Tx;
UUID_t UUID_Rx;

static char cmd[CMD_BUFF_SIZE];
static uint16_t cmd_buff_end = 0, cmd_buff_start = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : CHAT_DeviceInit.
* Description    : Init the Chat device.
* Input          : none.
* Return         : Status.
*******************************************************************************/
uint8_t CHAT_DeviceInit(void)
{
  uint8_t ret;
  uint16_t service_handle;
  uint16_t dev_name_char_handle;
  uint16_t appearance_char_handle;
  uint8_t name[] = {'B', 'l', 'u', 'e', 'N', 'R', 'G', '1'};
  
#if SERVER
  uint8_t role = GAP_PERIPHERAL_ROLE;
  uint8_t bdaddr[] = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02};
#else
  uint8_t role = GAP_CENTRAL_ROLE;
  uint8_t bdaddr[] = {0xbb, 0x00, 0x00, 0xE1, 0x80, 0x02};
#endif 
  
  /* Configure Public address */
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);
  if(ret != BLE_STATUS_SUCCESS){
    printf("Setting BD_ADDR failed: 0x%02x\r\n", ret);
    return ret;
  }

  /* Set the TX power to -2 dBm */
  aci_hal_set_tx_power_level(1, 4);

  /* GATT Init */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS) {
    printf ("Error in aci_gatt_init(): 0x%02x\r\n", ret);
    return ret;
  } else {
    printf ("aci_gatt_init() --> SUCCESS\r\n");
  }
  
  /* GAP Init */
  ret = aci_gap_init(role, 0x00, 0x08, &service_handle, 
                     &dev_name_char_handle, &appearance_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    printf ("Error in aci_gap_init() 0x%02x\r\n", ret);
    return ret;
  } else {
    printf ("aci_gap_init() --> SUCCESS\r\n");
  }

  /* Set the device name */
  ret = aci_gatt_update_char_value_ext(0,service_handle, dev_name_char_handle,0,sizeof(name),0, sizeof(name), name);
  if (ret != BLE_STATUS_SUCCESS) {
    printf ("Error in Gatt Update characteristic value 0x%02x\r\n", ret);
    return ret;
  } else {
    printf ("aci_gatt_update_char_value_ext() --> SUCCESS\r\n");
  }

#if  SERVER
  ret = Add_Chat_Service();
  if (ret != BLE_STATUS_SUCCESS) {
    printf("Error in Add_Chat_Service 0x%02x\r\n", ret);
    return ret;
  } else {
    printf("Add_Chat_Service() --> SUCCESS\r\n");
  }
  
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT     
  ret = OTA_Add_Btl_Service();
  if(ret == BLE_STATUS_SUCCESS)
    printf("OTA service added successfully.\n");
  else
    printf("Error while adding OTA service.\n");
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
  
#endif
  
  return BLE_STATUS_SUCCESS;
}

void Send_Data_Over_BLE(void)
{
  if(!APP_FLAG(SEND_DATA) || APP_FLAG(TX_BUFFER_FULL))
    return;
  
  while(cmd_buff_start < cmd_buff_end){
    uint32_t len = MIN(20, cmd_buff_end - cmd_buff_start);
    
#if SERVER
    if(aci_gatt_update_char_value_ext(connection_handle,chatServHandle,TXCharHandle,1,len,0, len,(uint8_t *)cmd+cmd_buff_start)==BLE_STATUS_INSUFFICIENT_RESOURCES){ 
#elif CLIENT
    if(aci_gatt_write_without_resp(connection_handle, rx_handle+1, len, (uint8_t *)cmd+cmd_buff_start)==BLE_STATUS_INSUFFICIENT_RESOURCES){       
#else
#error "Define SERVER or CLIENT"
#endif
      APP_FLAG_SET(TX_BUFFER_FULL);
      return;
    }
    cmd_buff_start += len;
  }
  
  // All data from buffer have been sent.
  APP_FLAG_CLEAR(SEND_DATA);
  cmd_buff_end = 0;
  NVIC_EnableIRQ(UART_IRQn);
}

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
  
  for (i = 0; i < Nb_bytes; i++)
  {
    if(cmd_buff_end >= CMD_BUFF_SIZE-1){
      cmd_buff_end = 0;
    }
    
    cmd[cmd_buff_end] = data_buffer[i];
    /* FIXME sort out somewhere to send the data */
    cmd_buff_end++;
    
    if(cmd[cmd_buff_end-1] == '\n'){
      if(cmd_buff_end != 1){
        
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
* Function Name  : Make_Connection.
* Description    : If the device is a Client create the connection. Otherwise puts
*                  the device in discoverable mode.
* Input          : none.
* Return         : none.
*******************************************************************************/
void Make_Connection(void)
{  
  tBleStatus ret;
  
#if CLIENT
  tBDAddr bdaddr = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02}; 
  
  ret = aci_gap_create_connection(0x4000, 0x4000, PUBLIC_ADDR, bdaddr, PUBLIC_ADDR, 40, 40, 0, 60, 2000 , 2000); 
  if (ret != BLE_STATUS_SUCCESS)
  {
    printf("Error while starting connection: 0x%04x\r\n", ret);
    Clock_Wait(100);        
  }
  
#else
  
  /* NOTE: Updated original Server advertising data in order to be also recognized by �BLE Sensor� app Client */
  
  tBDAddr bdaddr = {SERVER_ADDRESS};

  uint8_t manuf_data[MANUF_DATA_SIZE] = { 
    2,                      /* Length of AD type Transmission Power */
    0x0A, 0x00,             /* Transmission Power = 0 dBm */ 
    9,                      /* Length of AD type Complete Local Name */
    0x09,                   /* AD type Complete Local Name */ 
    LOCAL_NAME,             /* Local Name */            
    13,                     /* Length of AD type Manufacturer info */
    0xFF,                   /* AD type Manufacturer info */
    0x01,                   /* Protocol version */
    0x05,		            /* Device ID: 0x05 */
    0x00,                   /* Feature Mask byte#1 */
    0x00,                   /* Feature Mask byte#2 */
    0x00,                   /* Feature Mask byte#3 */
    0x00,                   /* Feature Mask byte#4 */
    0x00,                   /* BLE MAC start */
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00                    /* BLE MAC stop */
  };
  
  
  for (int var = 0; var < 6; ++var) {
    manuf_data[MANUF_DATA_SIZE -1  - var] = bdaddr[var];
  }
  
  uint8_t local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, LOCAL_NAME };

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  hci_le_set_scan_response_data(18,BTLServiceUUID4Scan); 
#else
  hci_le_set_scan_response_data(0,NULL);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 

  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                 sizeof(local_name), local_name, 0, NULL, 0, 0);
  if(ret != BLE_STATUS_SUCCESS)
  {
    printf ("Error in aci_gap_set_discoverable(): 0x%02x\r\n", ret);
  }
  else
  {
    printf ("aci_gap_set_discoverable() --> SUCCESS\r\n");
  }
  
  /* Update Advertising data with manuf_data */
  aci_gap_update_adv_data(MANUF_DATA_SIZE, manuf_data);
  
#endif
}

/*******************************************************************************
* Function Name  : APP_Tick.
* Description    : Tick to run the application state machine.
* Input          : none.
* Return         : none.
*******************************************************************************/
void APP_Tick(void)
{
#if CLIENT
  tBleStatus ret;
#endif
  
  if(APP_FLAG(SET_CONNECTABLE))
  {
    Make_Connection();
    APP_FLAG_CLEAR(SET_CONNECTABLE);
  }
  
  Send_Data_Over_BLE();

#if REQUEST_CONN_PARAM_UPDATE    
  if(APP_FLAG(CONNECTED) && !APP_FLAG(L2CAP_PARAM_UPD_SENT) && Timer_Expired(&l2cap_req_timer))
  {
    aci_l2cap_connection_parameter_update_req(connection_handle, 8, 16, 0, 600);
    APP_FLAG_SET(L2CAP_PARAM_UPD_SENT);
  }
#endif
  
#if CLIENT
  /* Start TX handle Characteristic discovery if not yet done */
  if (APP_FLAG(CONNECTED) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
  {
    if (!APP_FLAG(START_READ_TX_CHAR_HANDLE))
    {
      /* Discovery TX characteristic handle by UUID 128 bits */

      const uint8_t charUuid128_TX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};

      Osal_MemCpy(&UUID_Tx.UUID_16, charUuid128_TX, 16);
      ret = aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF,UUID_TYPE_128,&UUID_Tx);
      if (ret != 0) 
        printf ("Error in aci_gatt_disc_char_by_uuid() for TX characteristic: 0x%04xr\n", ret);
      else
        printf ("aci_gatt_disc_char_by_uuid() for TX characteristic --> SUCCESS\r\n");
      APP_FLAG_SET(START_READ_TX_CHAR_HANDLE);
    }
  }
  /* Start RX handle Characteristic discovery if not yet done */
  else if (APP_FLAG(CONNECTED) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
  {
    /* Discovery RX characteristic handle by UUID 128 bits */
    if (!APP_FLAG(START_READ_RX_CHAR_HANDLE))
    {
      /* Discovery TX characteristic handle by UUID 128 bits */

      const uint8_t charUuid128_RX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};
      
      Osal_MemCpy(&UUID_Rx.UUID_16, charUuid128_RX, 16);
      ret = aci_gatt_disc_char_by_uuid(connection_handle, 0x0001, 0xFFFF,UUID_TYPE_128,&UUID_Rx);
      if (ret != 0) 
        printf ("Error in aci_gatt_disc_char_by_uuid() for RX characteristic: 0x%04xr\n", ret);
      else
        printf ("aci_gatt_disc_char_by_uuid() for RX characteristic --> SUCCESS\r\n");
      
      APP_FLAG_SET(START_READ_RX_CHAR_HANDLE);
    }
  }
  
  if(APP_FLAG(CONNECTED) && APP_FLAG(END_READ_TX_CHAR_HANDLE) && APP_FLAG(END_READ_RX_CHAR_HANDLE) && !APP_FLAG(NOTIFICATIONS_ENABLED))
  {
    uint8_t client_char_conf_data[] = {0x01, 0x00}; // Enable notifications
    struct timer t;
    Timer_Set(&t, CLOCK_SECOND*10);
    
    while(aci_gatt_write_char_desc(connection_handle, tx_handle+2, 2, client_char_conf_data)==BLE_STATUS_NOT_ALLOWED){ //TX_HANDLE;
        // Radio is busy.
        if(Timer_Expired(&t)) break;
    }
    APP_FLAG_SET(NOTIFICATIONS_ENABLED);
  }
#endif
  
}/* end APP_Tick() */


/* ***************** BlueNRG-1 Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
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
  connection_handle = Connection_Handle;
  
  APP_FLAG_SET(CONNECTED);
  
#if REQUEST_CONN_PARAM_UPDATE
  APP_FLAG_CLEAR(L2CAP_PARAM_UPD_SENT);
  Timer_Set(&l2cap_req_timer, CLOCK_SECOND*2);
#endif
}/* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event occurs when a connection is terminated.
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
  APP_FLAG_CLEAR(TX_BUFFER_FULL);

  APP_FLAG_CLEAR(START_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(END_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(START_READ_RX_CHAR_HANDLE); 
  APP_FLAG_CLEAR(END_READ_RX_CHAR_HANDLE);

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  OTA_terminate_connection();
#endif 
  
}/* end hci_disconnection_complete_event() */


/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : This event occurs when an attribute is modified.
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
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  OTA_Write_Request_CB(Connection_Handle, Attr_Handle, Attr_Data_Length, Attr_Data);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
  
  Attribute_Modified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);      
}

#if CLIENT

/*******************************************************************************
 * Function Name  : aci_gatt_notification_event.
 * Description    : This event occurs when a notification is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[])
{ 
  uint16_t attr_handle;
 
  attr_handle = Attribute_Handle;
    if(attr_handle == tx_handle+1)
    {
      for(int i = 0; i < Attribute_Value_Length; i++) 
          printf("%c", Attribute_Value[i]);
    }
}

/*******************************************************************************
 * Function Name  : aci_gatt_disc_read_char_by_uuid_resp_event.
 * Description    : This event occurs when a discovery read characteristic by UUID response.
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
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
  {
    tx_handle = Attribute_Handle;
    printf("TX Char Handle 0x%04X\n", tx_handle);
  }
  else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
  {
    rx_handle = Attribute_Handle;
    printf("RX Char Handle 0x%04X\n", rx_handle);
  }
}

/*******************************************************************************
 * Function Name  : aci_gatt_proc_complete_event.
 * Description    : This event occurs when a GATT procedure complete is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle,
                                  uint8_t Error_Code)
{ 
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
  {
    printf("aci_GATT_PROCEDURE_COMPLETE_Event for START_READ_TX_CHAR_HANDLE \r\n");
    APP_FLAG_SET(END_READ_TX_CHAR_HANDLE);
  }
  else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
  {
    printf("aci_GATT_PROCEDURE_COMPLETE_Event for START_READ_TX_CHAR_HANDLE \r\n");
    APP_FLAG_SET(END_READ_RX_CHAR_HANDLE);
  }
}

#endif /* CLIENT */

/*******************************************************************************
 * Function Name  : aci_gatt_tx_pool_available_event.
 * Description    : This event occurs when a TX pool available is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{       
  /* It allows to notify when at least 2 GATT TX buffers are available */
  APP_FLAG_CLEAR(TX_BUFFER_FULL);
} 

/*******************************************************************************
 * Function Name  : aci_gatt_read_permit_req_event.
 * Description    : This event is given when a read request is received
 *                  by the server from the client.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
                                    uint16_t Attribute_Handle,
                                    uint16_t Offset)
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT   
  /* Lower/Higher Applications with OTA Service */
  aci_gatt_allow_read(Connection_Handle);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */ 
}

void aci_hal_end_of_radio_activity_event(uint8_t Last_State,
                                         uint8_t Next_State,
                                         uint32_t Next_State_SysTime)
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
  if (Next_State == 0x02) /* 0x02: Connection event slave */
  {
    OTA_Radio_Activity(Next_State_SysTime);  
  }
#endif 
}
