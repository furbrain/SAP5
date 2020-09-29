#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ble_const.h" 
#include "bluenrg1_stack.h"
#include "osal.h"
#include "app_state.h"
#include "chat.h"
#include "my_utils.h"

uint16_t chatServHandle, TXCharHandle, RXCharHandle;

static char cmd_buffer[40];
static char cmd[40];
static bool cmd_ready = false;

/* UUIDs */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

/*******************************************************************************
* Function Name  : Add_Chat_Service
* Description    : Add the Chat service.
* Input          : None
* Return         : Status.
*******************************************************************************/
uint8_t Add_Chat_Service(void)
{
  uint8_t ret;

  /*
  UUIDs: NRF protocol
  6e400001-b5a3-f393-e0a9-e50e24dcca9e uuid
  6e400002-b5a3-f393-e0a9-e50e24dcca9e Tx
  6e400003-b5a3-f393-e0a9-e50e24dcca9e Rx
  */

  const uint8_t uuid[16] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e};
  const uint8_t charUuidTX[16] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e};
  const uint8_t charUuidRX[16] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e};

  Osal_MemCpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 6, &chatServHandle); 
  if (ret != BLE_STATUS_SUCCESS) goto fail;    

  Osal_MemCpy(&char_uuid.Char_UUID_128, charUuidTX, 16);
  ret =  aci_gatt_add_char(chatServHandle, UUID_TYPE_128, &char_uuid, 20, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                16, 1, &TXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  Osal_MemCpy(&char_uuid.Char_UUID_128, charUuidRX, 16);
  ret =  aci_gatt_add_char(chatServHandle, UUID_TYPE_128, &char_uuid, 20, CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                16, 1, &RXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  //printf("Chat Service added.\nTX Char Handle %04X, RX Char Handle %04X\n", TXCharHandle, RXCharHandle);
  return BLE_STATUS_SUCCESS; 

fail:
  //printf("Error while adding Chat service.\n");
  return BLE_STATUS_ERROR ;
}

/*******************************************************************************
* Function Name  : Attribute_Modified_CB
* Description    : Callback when RX/TX attribute handle is modified.
* Input          : Handle of the characteristic modified. Handle of the attribute
*                  modified and data written
* Return         : None.
*******************************************************************************/
void Attribute_Modified_CB(uint16_t handle, uint16_t data_length, uint8_t *att_data)
{
	if(handle == RXCharHandle + 1)
	{
		for(int i = 0; i < data_length; i++) {
			if (att_data[i]=='\n') {
				strcpy(cmd, cmd_buffer);
				clearStr(cmd_buffer);
				cmd_ready = true;
			}
			else if (att_data[i] != 0) {
				appendChar(cmd_buffer, att_data[i], BUF_SIZE);
			}
		}
	  //printf("%c", att_data[i]);
	}
	else if(handle == TXCharHandle + 2)
	{
		if(att_data[0] == 0x01)
			APP_FLAG_SET(NOTIFICATIONS_ENABLED);
	}
}

const char* get_bt_text(void) {
	if (cmd_ready) {
		cmd_ready = false;
		return cmd;
	} else {
		return NULL;
	}
}

