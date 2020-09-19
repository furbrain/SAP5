/*
 * uart.h
 *
 *  Created on: 18 Sep 2020
 *      Author: phil
 */

#ifndef UART_H_
#define UART_H_
#include <stdbool.h>
void uart_init();
bool uart_receive_cmd(char *buffer, int maxlen);
void uart_send_response(const char *buffer, int maxlen);

#endif /* UART_H_ */
