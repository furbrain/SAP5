#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "unity.h"
#include "exception.h"
#include "CException.h"

#include "util_stub.h"
#include "mock_utils.h"
#include "mock_battery.h"
#include "mock_rtcc.h"

bool RTCC_TimeGet_Stub(struct tm *currentTime, int numcalls) {
    time_t t = time(NULL);
    localtime_r(&t, currentTime);
    return true;
}

void utils_turn_off_Stub(int32_t a, int numcalls) {
    THROW_WITH_REASON("Turned off", ERROR_UNSPECIFIED);
}

void delay_ms_Stub(int count, int numcalls) {
    usleep(count * 1000);
}

void util_setup() {
    wdt_clear_Ignore();
    delay_ms_StubWithCallback(delay_ms_Stub);
    delay_ms_safe_StubWithCallback(delay_ms_Stub);
    battery_get_units_IgnoreAndReturn(10);
    battery_get_voltage_IgnoreAndReturn(3.7);
    RTCC_TimeGet_StubWithCallback(RTCC_TimeGet_Stub);
    utils_turn_off_StubWithCallback(utils_turn_off_Stub);
}
