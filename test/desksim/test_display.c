#include "unity.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "mock_utils.h"
#include "mock_i2c1.h"
#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "i2c_stub.h"
#include "exception.h"

void suiteSetUp(void) {
    zmq_setup();
}

void delay_ms_stub(int count, int numcalls) {
    usleep(count*1000);
}

void setUp(void)
{
    delay_ms_safe_StubWithCallback(delay_ms_stub);
    delay_ms_StubWithCallback(delay_ms_stub);
    i2c_setup();
    display_init();
    display_flip(false);
}

void tearDown(void)
{
    usleep(2000000);
    display_clear_screen(true);
    display_off();
}

void test_i2c_fails(void) {
    char text[] = "broked";
    TEST_ASSERT_EQUAL_INT(write_i2c_block(0x10, text, 3),-1);
}

void test_send1(void) {
    TEST_ASSERT_EQUAL_INT(0, write_i2c_data2(0x3c,0x00,0xAD));
}

//void test_display(void)
//{
//    display_draw_line(0, 0, 30, 30);
//    display_show_buffer();
//}

//void test_display_write_multiline(void) {
//    display_write_multiline(0, "This is\nSome Multiline\nTest\nOf Doom", true);
//}

//void test_display_write_text(void) {
//    display_write_text(2, 0, "JustText", &large_font, false, true);
//}

void test_display_scroll_buffer(void) {
    int i;
    display_buf_t buf;
    memset(&buf[0][0], 0, 128*8);
    for (i=0; i<4; i++) {
        
        render_text_to_page(buf[i+2], i, 0, "scroll up", &large_font);
    }
    display_scroll_buffer(buf, true);

    memset(&buf[0][0], 0, 128*8);
    for (i=0; i<4; i++) {
        render_text_to_page(buf[i+2], i, 0, "down", &large_font);
    }
    display_scroll_buffer(buf, false);
}

//void test_display_inverted(void) {
//    display_write_text(2, 0, "Inverted", &large_font, false, true);
//    display_flip(true);
//}
    
