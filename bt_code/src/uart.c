#include <stdbool.h>
#include <string.h>
#include "bluenrg_x_device.h"
#include "BlueNRG1_conf.h"
#include "BlueNRG1_uart.h"
#include "BlueNRG1_sysCtrl.h"
#include "BlueNRG1_gpio.h"
#include "my_utils.h"

void uart_init(void)
{
  
  /* Clock enable */
  SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_UART | CLOCK_PERIPH_GPIO, ENABLE);
  
  /* Configure GPIO_Pin_8 and GPIO_Pin_11 as UART_TXD and UART_RXD*/
  GPIO_InitType GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = Serial1_Mode;
  GPIO_InitStructure.GPIO_Pull = DISABLE;
  GPIO_InitStructure.GPIO_HighPwr = DISABLE;
  GPIO_Init(&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = Serial1_Mode;
  GPIO_Init(&GPIO_InitStructure);

  /* ------------ USART configuration -------------------
  - BaudRate = 115200 baud  
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  UART_InitType UART_InitStructure;
  UART_InitStructure.UART_BaudRate = (uint32_t)UART_BAUDRATE;
  UART_InitStructure.UART_WordLengthTransmit = UART_WordLength_8b;
  UART_InitStructure.UART_WordLengthReceive = UART_WordLength_8b;
  UART_InitStructure.UART_StopBits = UART_StopBits_1;
  UART_InitStructure.UART_Parity = UART_Parity_No;
  UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
  UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;
  UART_InitStructure.UART_FifoEnable = ENABLE;
  UART_Init(&UART_InitStructure);
  
  /* Interrupt as soon as data is received. */
  UART_RxFifoIrqLevelConfig(FIFO_LEV_1_64);

  /* Enable UART */
  UART_Cmd(ENABLE);
}

bool uart_receive_cmd(char *buffer, int maxlen) {
	char c;
	while (UART_GetFlagStatus(UART_FLAG_RXFE)!=SET) {
		c = UART_ReceiveData();
		appendChar(buffer, c, maxlen);
		if (c=='\n') {
			return true;
		}
	}
	return false;
}

void uart_send_response(const char *buffer, int maxlen) {
	int len = strnlen(buffer, maxlen);
	for (int i=0; i < len; i++) {
		//make sure transmit buffer is empty
		while (UART_GetFlagStatus(UART_FLAG_TXFE) == RESET) {
		      ;
		}
		UART_SendData(buffer[i]);
	}
}
