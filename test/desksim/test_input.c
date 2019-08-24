#include "unity.h"
#include "mock_input.h"
#include "display.h"
#include "exception.h"
#include "mock_utils.h"
#include "mock_i2c1.h"
#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "i2c_stub.h"
#include "input_stub.h"
#include "i2c_util.h"
#include "font.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    i2c_setup();
    input_setup();
    delay_ms_safe_Ignore();
    delay_ms_Ignore();
    display_clear_screen(true);
    display_init();
    display_flip(false);
}

void tearDown(void)
{
    sleep(1);
}

void test_get_None(void) {
    display_write_multiline(3, "Press NOTHING", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(NONE, get_input());
}

void test_get_UP(void) {
    display_write_multiline(3, "Press FLIP UP", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(FLIP_UP, get_input());
}

void test_get_DOWN(void) {
    display_write_multiline(3, "Press FLIP DOWN", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(FLIP_DOWN, get_input());
}

void test_get_LEFT(void) {
    display_write_multiline(3, "Press FLIP LEFT", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(FLIP_LEFT, get_input());
}

void test_get_RIGHT(void) {
    display_write_multiline(3, "Press FLIP RIGHT", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(FLIP_RIGHT, get_input());
}

void test_get_SINGLE(void) {
    display_write_multiline(3, "Single Click", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(SINGLE_CLICK, get_input());
}

void test_get_DOUBLE(void) {
    display_write_multiline(3, "Double Click", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(DOUBLE_CLICK, get_input());
}

void test_get_LONG(void) {
    display_write_multiline(3, "Long Click", true);
    sleep(1);
    TEST_ASSERT_EQUAL_INT(LONG_CLICK, get_input());
}


