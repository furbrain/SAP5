#include <string.h>

#include "unity.h"
#include "exception.h"

#include "bt.h"
#include "config.h"

#include "mock_uart2.h"
#include "mock_memory.h"
#include "mock_display.h"
#include "mock_utils.h"


#define THROWS_ERROR_IF_BT_ABSENT(_code)                        \
	bool _bt_config_state = config.bt_present;                  \
	config.bt_present = false;                                  \
	TEST_ASSERT_THROWS(_code, ERROR_BLUETOOTH_NOT_PRESENT);     \
	config.bt_present = _bt_config_state;


void setUp(void)
{
	config.bt_present = true;
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

void expect_enter(void) {
	UART2_ReceiveBufferClear_Expect();
	expect_write("$$$");
	delay_ms_safe_Expect(10);
	expect_read("CMD>");
}

void expect_leave(void) {
	UART2_ReceiveBufferClear_Expect();
	expect_write("---\r");
	delay_ms_safe_Expect(50);
	expect_read("END\r");
}

#define expect_command(cmd, timeout) expect_command_raw(cmd "\r", timeout)

void expect_command_raw(const char* cmd, unsigned int timeout) {
	UART2_ReceiveBufferClear_Expect();
	expect_write(cmd);
	delay_ms_safe_Expect(timeout);
	expect_read("AOK\r");
}

void test_bt_init(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_init());
	UART2_Initialize_Expect();
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

void test_bt_enter_command_mode(void)
{
	expect_enter();
	bt_enter_command_mode();
}

void test_bt_enter_command_mode_failed(void)
{
	UART2_ReceiveBufferClear_Expect();
	expect_write("$$$");
	delay_ms_safe_Expect(10);
	expect_read("gibbers");
	TEST_ASSERT_THROWS(bt_enter_command_mode(), ERROR_BLUETOOTH_DID_NOT_RESPOND);
}

void test_bt_enter_command_mode_bt_absent(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_enter_command_mode());
}

void test_bt_send_command(void)
{
	expect_command("S-,Shetland", 10);
	bt_send_command("S-,Shetland",10);	
}

void test_bt_send_command_failed(void)
{
	UART2_ReceiveBufferClear_Expect();
	expect_write("blib\r");
	delay_ms_safe_Expect(10);
	expect_read("ERR\r");
	TEST_ASSERT_THROWS(bt_send_command("blib",10), ERROR_BLUETOOTH_COMMAND_FAILED);	
}

void test_bt_send_command_bt_absent(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_send_command("S-,Shetland",10));
}

void test_bt_leave_command_mode(void)
{
	expect_leave();
	bt_leave_command_mode();
}

void test_bt_leave_command_mode_fail(void)
{
	UART2_ReceiveBufferClear_Expect();
	expect_write("---\r");
	delay_ms_safe_Expect(50);
	expect_read("");
	TEST_ASSERT_THROWS(bt_leave_command_mode(), ERROR_BLUETOOTH_COMMAND_FAILED);
}

void test_bt_leave_command_mode_bt_absent(void)
{
	THROWS_ERROR_IF_BT_ABSENT(bt_leave_command_mode());
}


void test_bt_beep(void)
{
	struct test_field {
		double f;
		const char* text;
	};
	struct test_field test_cases[] = {
		{523.251, "[,1,2,07A4,03D2\r"},
		{659.255, "[,1,2,0611,0308\r"},
		{783.991, "[,1,2,051A,028D\r"},
		{1046.5, "[,1,2,03D2,01E9\r"}
	};
	int i;
	for (i=0;i<4;i++) {
		expect_command_raw(test_cases[i].text, 10);
		bt_beep(test_cases[i].f);
	}
}

void test_bt_reset(void)
{
	expect_enter();
	expect_command("SF,1",500); //reset to factory settings
	expect_command("S-,Shetland",50); //set name
	expect_command("SS,C0",20); //enable correct services
	expect_leave();
	bt_reset();
}

void test_bt_reset_bt_absent(void) {
	THROWS_ERROR_IF_BT_ABSENT(bt_reset());
}

void test_bt_advertise(void)
{
	expect_enter();
	expect_command("A,0050,005E",500);
	expect_leave();
	bt_advertise();
}

void test_bt_send_data(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement bt send data");
}

