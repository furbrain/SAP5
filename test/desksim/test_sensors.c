#include "unity.h"
#include "exception.h"
#include "sensors.h"
#include "i2c_util.h"
#include "version.h"
//#include "calibrate.h"
#include "maths.h"
#include "eigen3x3.h"
#include "mock_utils.h"
#include "mock_i2c1.h"
#include "mock_memory.h"
#include "mock_display.h"

#include <zmq.h>
#include <unistd.h>
#include "zmqstubs.h"
#include "i2c_stub.h"
#include "config.h"

void suiteSetUp(void) {
    zmq_setup();
}

void setUp(void)
{
    config = default_config;
    i2c_setup();
    wdt_clear_Ignore();
    delay_ms_safe_Ignore();
    sensors_init();
}

void tearDown(void)
{
    sleep(1);
}

void test_raw_sensors(void)
{
    struct RAW_SENSORS sensors;
    sensors_read_raw(&sensors);
    printf("Mag: {%d,%d,%d}\n", sensors.mag[0], sensors.mag[1], sensors.mag[2]);
    printf("Accel: {%d,%d,%d}\n", sensors.accel[0], sensors.accel[1], sensors.accel[2]);
}


void test_uncalibrated(void)
{
    struct COOKED_SENSORS sensors;
    sensors_read_uncalibrated(&sensors,2);
    printf("UMag: {%f,%f,%f}", sensors.mag[0], sensors.mag[1], sensors.mag[2]);
    printf("UAccel: {%f,%f,%f}", sensors.accel[0], sensors.accel[1], sensors.accel[2]);
}

void test_cooked_sensors(void)
{
    struct COOKED_SENSORS sensors;
    sensors_read_cooked(&sensors,2);
    printf("Mag: {%f,%f,%f}", sensors.mag[0], sensors.mag[1], sensors.mag[2]);
    printf("Accel: {%f,%f,%f}", sensors.accel[0], sensors.accel[1], sensors.accel[2]);
}
