#include "unity.h"
#include "mock_rtcc.h"
#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "time_stub.h"
#include "exception.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    time_setup();
}

void tearDown(void)
{
}

void test_get_time(void) {
    struct tm the_time;
    char text[100];
    RTCC_TimeGet(&the_time);
    strftime(text, 100, "%c", &the_time);
    printf("Time: %s\n", text);
}

