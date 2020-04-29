#include <zmq.h>
#include <unistd.h>
#include "unity.h"

#include "mock_utils.h"
#include "mock_i2c1.h"
#include "mock_input.h"

#include "zmqstubs.h"
#include "i2c_stub.h"
#include "input_stub.h"


#include "exception.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "ui.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
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

void test_simple(void) {
    struct UI_MULTI_SELECT test  = {
        "2345", 30, 
        {{1235, 1234, 2345, 4, 0, NULL}
        }};
    ui_multi_select(&test);
};

void test_choose_single_line(void) {
    ui_yes_no("Single");
}

void test_choose_double_line(void) {
    ui_yes_no("Double\nLine");
}
