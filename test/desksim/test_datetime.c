#include <zmq.h>
#include <unistd.h>
#include "unity.h"

#include "mock_utils.h"
#include "mock_i2c1.h"
#include "mock_input.h"
#include "mock_rtcc.h"

#include "zmqstubs.h"
#include "i2c_stub.h"
#include "input_stub.h"


#include "exception.h"
#include "display.h"
#include "i2c_util.h"
#include "font.h"
#include "ui.h"
#include "datetime.h"

struct tm tm_out;

void suiteSetUp(void) {
    zmq_setup();
}

bool RTCC_TimeGet_Stub(struct tm *currentTime, int numcalls) {
    time_t t = time(NULL);
    localtime_r(&t, currentTime);
    return true;
}

void RTCC_TimeSet_Stub(struct tm *currentTime, int numcalls) {
    char text[40];
    strftime(text, 40, "Time set to: %Y-%m-%d %H:%M\n", currentTime);
    printf("%s", text);
}


void setUp(void)
{
    delay_ms_Ignore();
    delay_ms_safe_Ignore();
    i2c_setup();
    input_setup();
    display_init();
    display_flip(false);
    RTCC_TimeGet_StubWithCallback(RTCC_TimeGet_Stub);
    RTCC_TimeSet_StubWithCallback(RTCC_TimeSet_Stub);
}

void tearDown(void)
{
    sleep(1);
}

void test_set_date(void)
{
    datetime_set_date(0);
}

void test_set_time(void)
{
    datetime_set_time(0);
}
