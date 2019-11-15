#include "unity.h"
#include "beep.h"
#include "exception.h"
#include "mock_utils.h"
#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "beep_stub.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    delay_ms_safe_Ignore();
    beep_setup();
}

void tearDown(void)
{
    sleep(1);
}

void test_beep(void)
{
    beep(1000,1000);
}

void test_beep_happy(void)
{
    beep_happy();
}

void test_beep_sad(void)
{
    beep_sad();
}

void test_beep_beep(void)
{
    beep_beep();
}
