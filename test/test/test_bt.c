#include <string.h>

#include "unity.h"
#include "exception.h"

#include "bt.h"
#include "config.h"

#include "mock_uart2.h"
#include "mock_memory.h"
#include "mock_display.h"
#include "mock_utils.h"
#include "mock_version.h"

bool bt_present;

#define THROWS_ERROR_IF_BT_ABSENT(_code)                        \
	bool _bt_config_state = bt_present;                  \
	bt_present = false;                                  \
	TEST_ASSERT_THROWS(_code, ERROR_BLUETOOTH_NOT_PRESENT);     \
	bt_present = _bt_config_state;


void setUp(void)
{
	bt_present = true;
    UART2_ReceiveBufferClear_Ignore();
    delay_ms_safe_Ignore();
}

void tearDown(void)
{
}

void expect_write(const char* text) {
	UART2_WriteBuffer_ExpectAndReturn(text, strlen(text), strlen(text));
}

void expect_read(char* text) {
	UART2_TransferStatusGet_ExpectAndReturn(UART2_TRANSFER_STATUS_RX_DATA_PRESENT);
	UART2_ReadBuffer_ExpectAndReturn(NULL,0,strlen(text));
	UART2_ReadBuffer_IgnoreArg_buffer();
	UART2_ReadBuffer_IgnoreArg_numbytes();
	UART2_ReadBuffer_ReturnMemThruPtr_buffer(text,strlen(text));
	UART2_TransferStatusGet_ExpectAndReturn(UART2_TRANSFER_STATUS_RX_EMPTY);
}




#define expect_command(cmd, response, timeout) expect_write(cmd "\n"); expect_read(response);

void test_bt_init_when_absent(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_init());
}

void test_bt_init(void) {
    struct VERSION_ID id = {"Angry", "Gorilla"};
	UART2_Initialize_Expect();
    version_get_id_ExpectAndReturn(id);
    expect_command("EYUP AG", "OWDO", 10);
	bt_init();
}

void test_bt_write_str(void) {
	UART2_WriteBuffer_ExpectAndReturn("1234",4,4);
	bt_write_str("1234");
}

void test_bt_write_str_buffer_overflow(void) {
	UART2_WriteBuffer_ExpectAndReturn("1234",4,2);
	TEST_ASSERT_THROWS(bt_write_str("1234"),ERROR_BLUETOOTH_BUFFER_OVERFLOW);
}

void test_bt_read_str(void) {
	char buffer[10] = "123456778";
	expect_read("CMD>");
	bt_read_str(buffer, 10);
	TEST_ASSERT_EQUAL_STRING("CMD>", buffer);
}

void test_bt_read_str_throws_if_recent_overflow(void) {
	char buffer[10] = "123456778";
	UART2_TransferStatusGet_ExpectAndReturn(UART2_TRANSFER_STATUS_RX_FULL);
	TEST_ASSERT_THROWS(bt_read_str(buffer, 10),ERROR_BLUETOOTH_BUFFER_OVERFLOW);
}

void test_bt_read_str_throws_if_unable_to_empty_buffer(void) {
	char buffer[10] = "123456778";
	UART2_TransferStatusGet_ExpectAndReturn(UART2_TRANSFER_STATUS_RX_DATA_PRESENT);
	UART2_ReadBuffer_ExpectAndReturn(NULL,0,4);
	UART2_ReadBuffer_IgnoreArg_buffer();
	UART2_ReadBuffer_IgnoreArg_numbytes();
	UART2_ReadBuffer_ReturnMemThruPtr_buffer("CMD>",4);
	UART2_TransferStatusGet_ExpectAndReturn(UART2_TRANSFER_STATUS_RX_DATA_PRESENT);
	TEST_ASSERT_THROWS(bt_read_str(buffer, 10),ERROR_BLUETOOTH_BUFFER_OVERFLOW);
}




void test_bt_send_command(void)
{
	expect_command("EYUP JC", "OWDO", 10);
	bt_send_command("EYUP JC", "OWDO", 10);
}

void test_bt_send_command_failed(void)
{
	UART2_ReceiveBufferClear_Expect();
	expect_write("blib\n");
	delay_ms_safe_Expect(10);
	expect_read("ERR");
	TEST_ASSERT_THROWS(bt_send_command("blib", "fred", 10), ERROR_BLUETOOTH_COMMAND_FAILED);	
}

void test_bt_send_command_bt_absent(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_send_command("EYUP", "OWDO", 10));
}
