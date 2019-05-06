#include <string.h>

#include "bt.h"
#include "config.h"
#include "exception.h"
#include "utils.h"
#include "mcc_generated_files/uart2.h"

void check_bt_present(void) {
	if (!config.bt_present) {
		THROW_WITH_REASON("Bluetooth device not present",ERROR_BLUETOOTH_NOT_PRESENT);
	}
}

TESTABLE_STATIC
void bt_write_str(const char* text) {
	unsigned int bytes_sent;
	unsigned int text_length;
	text_length =  strlen(text);
	bytes_sent = UART2_WriteBuffer(text,text_length);
	if (bytes_sent < text_length) {
		THROW_WITH_REASON("Bluetooth send buffer overflow", ERROR_BLUETOOTH_BUFFER_OVERFLOW);
	}
}

TESTABLE_STATIC
void bt_read_str(char* buffer, unsigned int max_len) {
	unsigned int num_read;
	UART2_TRANSFER_STATUS status;
	status = UART2_TransferStatusGet();
	if (status & UART2_TRANSFER_STATUS_RX_FULL) {
		THROW_WITH_REASON("Bluetooth receive buffer overflow", ERROR_BLUETOOTH_BUFFER_OVERFLOW);
	}
	num_read = UART2_ReadBuffer(buffer, max_len-1);
	status = UART2_TransferStatusGet();
	if (status & UART2_TRANSFER_STATUS_RX_DATA_PRESENT) {
		THROW_WITH_REASON("Bluetooth read did not empty buffer", ERROR_BLUETOOTH_BUFFER_OVERFLOW);
	}
	buffer[num_read] = '\0';
}
		

void bt_init(void) {
	check_bt_present();
	UART2_Initialize();
}

void bt_enter_command_mode(void) {
	char buffer[10];
	check_bt_present();
	UART2_ReceiveBufferClear();
	bt_write_str("$$$");
	delay_ms_safe(10);
	bt_read_str(buffer,10);
	if (strcmp(buffer,"CMD>")) {
		THROW_WITH_REASON("Unable to enter command mode", ERROR_BLUETOOTH_DID_NOT_RESPOND);
	}
}

void bt_send_command(const char* cmd, unsigned int timeout) {
	char buffer[30];
	SAFE_STRING_COPY(buffer, cmd, 26);
	strcat(buffer,"\r");
	check_bt_present();
	UART2_ReceiveBufferClear();
	bt_write_str(buffer);
	delay_ms_safe(timeout);
	bt_read_str(buffer,26);
	if(strcmp(buffer,"AOK\r")) {
		THROW_WITH_REASON(cmd, ERROR_BLUETOOTH_COMMAND_FAILED);
	}	
}

void bt_leave_command_mode(void) {
	char buffer[10];
	check_bt_present();
	UART2_ReceiveBufferClear();
	bt_write_str("---\r");
	delay_ms_safe(50);
	bt_read_str(buffer, 10);
	if (strcmp(buffer, "END\r")) {
		THROW_WITH_REASON("Leave command mode", ERROR_BLUETOOTH_COMMAND_FAILED);
	}
}
