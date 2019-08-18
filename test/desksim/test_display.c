#include "unity.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "mock_utils.h"
#include "mock_i2c1.h"
#include <zmq.h>
#include "zmqstubs.h"

void setUp(void)
{
    delay_ms_safe_Ignore();
    delay_ms_Ignore();
    i2c_setup();
    display_init();
}

void tearDown(void)
{
    //display_off();
}

void test_display(void)
{
    display_on();
    display_draw_line(0, 0, 30, 30);
    display_show_buffer();
}
