
#ifndef _CHAT_H_
#define _CHAT_H_

uint8_t CHAT_DeviceInit(void);
void Process_InputData(uint8_t* data_buffer, uint16_t Nb_bytes);
void APP_Tick(void);

#endif
