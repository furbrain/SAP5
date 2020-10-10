#include <string.h>

#include "bt.h"
#include "config.h"
#include "exception.h"
#include "utils.h"
#include "version.h"
#include "mcc_generated_files/uart2.h"
bool bt_present = false;

void check_bt_present(void) {
	if (!bt_present) {
		THROW_WITH_REASON("Bluetooth device not present",ERROR_BLUETOOTH_NOT_PRESENT);
	}
}

TESTABLE_STATIC
void bt_write_str(const char* text) {
	unsigned int bytes_sent;
	unsigned int text_length;
	text_length =  strlen(text);
	bytes_sent = UART2_WriteBuffer((uint8_t*)text, text_length);
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
	num_read = UART2_ReadBuffer((uint8_t*)buffer, max_len-1);
	status = UART2_TransferStatusGet();
	if (status & UART2_TRANSFER_STATUS_RX_DATA_PRESENT) {
		THROW_WITH_REASON("Bluetooth read did not empty buffer", ERROR_BLUETOOTH_BUFFER_OVERFLOW);
	}
	buffer[num_read] = '\0';
}
		

void bt_init(void) {
    struct VERSION_ID id;
    char buffer[20];
    check_bt_present();
	UART2_Initialize();
    id = version_get_id();
    sprintf(buffer, "EYUP %c%c", *id.adjective, *id.animal);
    bt_send_command(buffer, "OWDO", 20);
}


void bt_send_command(const char* cmd, const char *expected_response, unsigned int timeout) {
	char buffer[50];
	SAFE_STRING_COPY(buffer, cmd, 50);
	strcat(buffer,"\n");
	check_bt_present();
	UART2_ReceiveBufferClear();
	bt_write_str(buffer);
	delay_ms_safe(timeout);
	bt_read_str(buffer,50);
	if(strncmp(buffer, "OOECK", 5)==0) {
		THROW_WITH_REASON(&buffer[6], ERROR_BLUETOOTH_COMMAND_FAILED);
	}
    if(strncmp(buffer, expected_response, strlen(expected_response))!=0) {
    	SAFE_STRING_COPY(buffer, cmd, 50);
		THROW_WITH_REASON(buffer, ERROR_BLUETOOTH_COMMAND_FAILED);        
    }
}

void bt_send_packet(const char* packet, int length) {
    char buffer[50] = "SEZYU ";
    int i = 0;
    while (i < length) {
        sprintf(&buffer[6+i*2], "%02hhx", (uint8_t) *packet++);
        i++;
    }
    bt_send_command(buffer, "ALLSED", 30);
}

void bt_get_response(const char *cmd, char* response, int maxlen, int timeout) {
    char buffer[50];
    int resp_len;
	SAFE_STRING_COPY(buffer, cmd, 50);
	strcat(buffer,"\n");
	check_bt_present();
	UART2_ReceiveBufferClear();
	bt_write_str(buffer);
	delay_ms_safe(timeout);
	bt_read_str(response,maxlen);
    resp_len = strlen(response);
    if (response[resp_len] == '\n') {
        response[resp_len] = 0;
    }
}

void bt_beep_start(int freq) {
	char buffer[20];
	check_bt_present();
    sprintf(buffer, "BEEP %d", freq);
	bt_send_command(buffer, "BEEPIN", 10);
}

void bt_beep_stop(void) {
	check_bt_present();
	bt_send_command("HUSH", "HUSHIN", 10);
}

bool bt_connected(void) {
    char response[10];
    bt_get_response("NATTERIN?", response, 10, 10);
    return (strncmp(response, "AYE", 3) == 0);
}
