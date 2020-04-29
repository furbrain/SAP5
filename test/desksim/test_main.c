#include <zmq.h>
#include <unistd.h>
#include "unity.h"

#include "mock_utils.h"
#include "mock_i2c1.h"
#include "mock_input.h"
#include "mock_uart1.h"
#include "mock_interrupt_manager.h"
#include "mock_debug.h"
#include "mock_memory.h"
#include "mock_battery.h"
#include "mock_rtcc.h"

#include "zmqstubs.h"
#include "beep_stub.h"
#include "adc_stub.h"
#include "i2c_stub.h"
#include "input_stub.h"
#include "uart_stub.h"
#include "debug_stub.h"
#include "util_stub.h"


#include "beep.h"
#include "exception.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "sensors.h"
#include "config.h"
#include "menu.h"
#include "version.h"
#include "laser.h"
#include "measure.h"
#include "maths.h"
#include "calibrate.h"
#include "interface.h"
#include "survey.h"
#include "leg.h"
#include "images.h"
#include "visualise.h"
#include "eigen3x3.h"
#include "model.h"
#include "datetime.h"
#include "ui.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    config = default_config;
    beep_setup();
    i2c_setup();
    input_setup();
    uart1_setup();
    util_setup();
    sensors_init();
}

void tearDown(void)
{
    sleep(1);
}

void test_ui(void)
{
    TEST_ASSERT_THROWS(measure(), ERROR_UNSPECIFIED);
}

