#include <zmq.h>
#include <unistd.h>
#include "unity.h"

#include "mock_utils.h"
#include "mock_i2c1.h"
#include "mock_input.h"
#include "mock_leg.h"
#include "mock_menu.h"
#include "mock_interface.h"
#include "mock_memory.h"
#include "mock_sensors.h"
#include "survey.h"
#include "mock_beep.h"
#include "mock_laser.h"
#include "images.h"
#include "zmqstubs.h"
#include "i2c_stub.h"
#include "input_stub.h"


#include "exception.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "ui.h"
#include "measure.h"
#include "config.h"

struct tm tm_out;

DECLARE_MENU(main_menu, {

});

void suiteSetUp(void) {
    zmq_setup();
}


void setUp(void)
{
    config = default_config;
    delay_ms_Ignore();
    delay_ms_safe_Ignore();
    i2c_setup();
    input_setup();
    display_init();
    display_flip(false);
}

void tearDown(void)
{
    sleep(1);
}

void test_test_storage_1(void)
{
    custom_storage(3);
}

void test_test_storage_29(void)
{
    custom_storage(29);
}
