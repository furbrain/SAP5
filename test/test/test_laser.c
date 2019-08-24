#include "unity.h"
#include "exception.h"
#include "CException.h"
#include "laser.h"
#include "config.h"
#include "mock_uart1.h"
#include "mock_utils.h"
#include "mock_memory.h"
#include "mock_display.h"

void test_laser_on(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('O');
    laser_on();
    
}

void test_laser_off(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('C');
    laser_off();
}

void test_laser_start_fast(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('F');
    laser_start(LASER_FAST);
}

void test_laser_start_medium(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('D');
    laser_start(LASER_MEDIUM);
}

void test_laser_start_slow(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('M');
    laser_start(LASER_SLOW);
}
void test_laser_result_ready(void) {
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(22);
    TEST_ASSERT_FALSE(laser_result_ready());
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(18);
    UART1_Peek_ExpectAndReturn(4,'f');
    TEST_ASSERT_FALSE(laser_result_ready());
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(10);
    UART1_Peek_ExpectAndReturn(4,'.');
    TEST_ASSERT_TRUE(laser_result_ready());
}

void test_laser_get_result(void) {
    UART1_ReadBuffer_ExpectAndReturn(NULL,15,9);
    UART1_ReadBuffer_IgnoreArg_buffer();
    UART1_ReadBuffer_ReturnArrayThruPtr_buffer(": 3.127m", 9);
    TEST_ASSERT_EQUAL_DOUBLE(3.127, laser_get_result());   
}

void test_laser_get_result_fails(void) {
    UART1_ReadBuffer_ExpectAndReturn(NULL,15,9);
    UART1_ReadBuffer_IgnoreArg_buffer();
    UART1_ReadBuffer_ReturnArrayThruPtr_buffer("gibbersh", 9);
    TEST_ASSERT_THROWS(laser_get_result(), ERROR_LASER_READ_FAILED);   
}

void test_laser_read_normal(void) {
    config.calib.laser_offset = 0.09;
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('D');
    delay_ms_safe_Ignore();
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(22);
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(22);
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(22);
    UART1_ReceiveBufferSizeGet_ExpectAndReturn(10);
    UART1_Peek_ExpectAndReturn(4,'.');
    UART1_ReadBuffer_ExpectAndReturn(NULL,15,9);
    UART1_ReadBuffer_IgnoreArg_buffer();
    UART1_ReadBuffer_ReturnArrayThruPtr_buffer(": 3.127m", 9);
    TEST_ASSERT_EQUAL_DOUBLE(3.217, laser_read(LASER_MEDIUM, 1000));
}

void test_laser_read_timeout(void) {
    UART1_ReceiveBufferClear_Expect();
    UART1_Write_Expect('D');
    UART1_ReceiveBufferSizeGet_IgnoreAndReturn(22);
    delay_ms_safe_Ignore();
    TEST_ASSERT_THROWS(laser_read(LASER_MEDIUM,1000), ERROR_LASER_READ_FAILED);
}
