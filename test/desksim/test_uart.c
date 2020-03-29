#include "unity.h"
#include "laser.h"
#include "exception.h"
#include "mock_utils.h"
#include "mock_uart1.h"
#include "mock_memory.h"
#include "mock_display.h"

#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "uart_stub.h"
#include "config.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    uart1_setup();
    delay_ms_safe_Ignore();
}

void tearDown(void)
{
    sleep(1);
}

void test_laser(void)
{
    int result;
    result = laser_read(LASER_MEDIUM,1000);
    printf("Result %d", result);
}


