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

#define CMD_BUFF_SIZE 64

#define SERVER_ADDRESS 0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02
#define LOCAL_NAME  'C','h','a','t','_','1','_','2'
#define MANUF_DATA_SIZE 39
#define NAME_MAX_LEN 22
/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint8_t connInfo[20];
volatile int app_flags = 0;
volatile uint16_t connection_handle = 0;
static char name[NAME_MAX_LEN] = "Shetland";

/** 
  * @brief  Handle of TX,RX  Characteristics.
  */ 

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
  
  uint8_t role = GAP_PERIPHERAL_ROLE;
  uint8_t bdaddr[] = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02};
  
  /* Configure Public address */
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);
  if(ret != BLE_STATUS_SUCCESS){
    //printf("Setting BD_ADDR failed: 0x%02x\r\n", ret);
    return ret;
  }

  /* Set the TX power to -2 dBm */
  aci_hal_set_tx_power_level(1, 4);

  /* GATT Init */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS) {
    //printf ("Error in aci_gatt_init(): 0x%02x\r\n", ret);
    return ret;
  } else {
    //printf ("aci_gatt_init() --> SUCCESS\r\n");
  }
  
  /* GAP Init */
  ret = aci_gap_init(role, 0x00, 0x08, &service_handle, 
                     &dev_name_char_handle, &appearance_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    //printf ("Error in aci_gap_init() 0x%02x\r\n", ret);
    return ret;
  } else {
    //printf ("aci_gap_init() --> SUCCESS\r\n");
  }

  /* Set the device name */
  ret = aci_gatt_update_char_value_ext(0,service_handle, dev_name_char_handle,0,sizeof(name),0, sizeof(name), name);
  if (ret != BLE_STATUS_SUCCESS) {
    //printf ("Error in Gatt Update characteristic value 0x%02x\r\n", ret);
    return ret;
  } else {
    //printf ("aci_gatt_update_char_value_ext() --> SUCCESS\r\n");
  }

  ret = Add_Chat_Service();
  if (ret != BLE_STATUS_SUCCESS) {
    //printf("Error in Add_Chat_Service 0x%02x\r\n", ret);
    return ret;
  } else {
    //printf("Add_Chat_Service() --> SUCCESS\r\n");
  }
  
  ret = aci_gap_set_io_capability(IO_CAP_DISPLAY_ONLY);
  if (ret != BLE_STATUS_SUCCESS) {
    //printf("Error in Set IO Cap 0x%02x\r\n", ret);
    return ret;
  }

  ret = aci_gap_set_authentication_requirement(BONDING,
                                               MITM_PROTECTION_REQUIRED,
                                               SC_IS_NOT_SUPPORTED,
                                               KEYPRESS_IS_NOT_SUPPORTED,
                                               0x07,
                                               0x10,
                                               USE_FIXED_PIN_FOR_PAIRING,
                                               0x00,
                                               0x00);
  if (ret != BLE_STATUS_SUCCESS) {
    //printf("Error in Authentication 0x%02x\r\n", ret);
    return ret;
  }
  return BLE_STATUS_SUCCESS;
}

void Set_ID_And_Set_Discoverable(const char *id) {
	snprintf(name, NAME_MAX_LEN, "Shetland_%s", id);
	APP_FLAG_SET(SET_CONNECTABLE);
}

void Send_Data_Over_BLE(void)
{
  if(!APP_FLAG(SEND_DATA) || APP_FLAG(TX_BUFFER_FULL))
    return;

  while(cmd_buff_start < cmd_buff_end){
    uint32_t len = MIN(20, cmd_buff_end - cmd_buff_start);

    if(aci_gatt_update_char_value_ext(connection_handle,chatServHandle,TXCharHandle,1,len,0, len,(uint8_t *)cmd+cmd_buff_start)==BLE_STATUS_INSUFFICIENT_RESOURCES){
      APP_FLAG_SET(TX_BUFFER_FULL);
      return;
    }
    cmd_buff_start += len;
  }

  // All data from buffer have been sent.
  APP_FLAG_CLEAR(SEND_DATA);
  cmd_buff_end = 0;
}

/*******************************************************************************
* Function Name  : Process_InputData.
* Description    : Process a command. It should be called when data are received.
* Input          : data_buffer: data address.
*	           Nb_bytes: number of received bytes.
* Return         : none.
*******************************************************************************/
void Process_InputData(const char* data, uint16_t Nb_bytes)
{
  if (Nb_bytes>CMD_BUFF_SIZE-1) {
      memcpy(cmd, data, CMD_BUFF_SIZE-1);
      cmd_buff_end = CMD_BUFF_SIZE-1;
  } else {
      memcpy(cmd, data, Nb_bytes);
      cmd_buff_end = Nb_bytes;
  }
		// Set flag to send data. Disable UART IRQ to avoid overwriting buffer with new incoming data
  APP_FLAG_SET(SEND_DATA);
  cmd_buff_start = 0;
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
  int name_len = strlen(name);
  uint8_t name_data[NAME_MAX_LEN+1] = {0x09,0x00};
  
  /* add name */
  name_data[0] = 0x09; /* AD type complet local name */
  memcpy(&name_data[1], name, name_len);

  hci_le_set_scan_response_data(0,NULL);

  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                 name_len+1, name_data, 0, NULL, 0, 0);
  if(ret != BLE_STATUS_SUCCESS)
  {
    //printf ("Error in aci_gap_set_discoverable(): 0x%02x\r\n", ret);
  }
  else
  {
    //printf ("aci_gap_set_discoverable() --> SUCCESS\r\n");
  }
  
}

/*******************************************************************************
* Function Name  : APP_Tick.
* Description    : Tick to run the application state machine.
* Input          : none.
* Return         : none.
*******************************************************************************/
void APP_Tick(void)
{
  
  if(APP_FLAG(SET_CONNECTABLE))
  {
    Make_Connection();
    APP_FLAG_CLEAR(SET_CONNECTABLE);
  }
  
  Send_Data_Over_BLE();

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
  
  Attribute_Modified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);      
}


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
