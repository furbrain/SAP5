
#ifndef _CHAT_H_
#define _CHAT_H_

uint8_t CHAT_DeviceInit(void);
void APP_Tick(void);
void Process_InputData(const char* data_buffer, uint16_t Nb_bytes);
void Set_ID_And_Set_Discoverable(const char *id);

#endif // _CHAT_H_
