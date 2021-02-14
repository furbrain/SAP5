#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics.h>
#include <xc.h>
#include "maths.h"
#include "calibrate.h"
#include "exception.h"
#include "eigen3x3.h"
#include "gsl_static.h"
#include "config.h"
#include "font.h"
#include "leg.h"

#include "mag_sample_data2.inc"
#include "mock_sensors.h"
#include "mock_survey.h"
#include "mock_display.h"
#include "mock_utils.h"
#include "mock_laser.h"
#include "mock_memory.h"
#include "mock_beep.h"
#include "mock_input.h"
#include "mock_ui.h"


#define DEGREES_PER_RADIAN 57.296

struct SURVEY survey_current;

void suiteSetUp(void) {
    exception_init();
}

void write_multiline(int page,const char* text, bool immediate, int num_calls) {
    printf("DISPLAY: %s\n", text);
}

void sensors_read(struct COOKED_SENSORS *sensors, int count, int num_calls) {
    memcpy(sensors->mag, &mag_sample_data[num_calls*3], sizeof(double)*3);
    memcpy(sensors->accel, &grav_sample_data[num_calls*3], sizeof(double)*3);
}


void setUp(void) {
    display_clear_Ignore();
    display_write_multiline_StubWithCallback(write_multiline);
    display_show_buffer_Ignore();
    delay_ms_safe_Ignore();
    wdt_clear_Ignore();
    beep_beep_Ignore();
    beep_happy_Ignore();
    laser_on_Ignore();
    laser_off_Ignore();
    display_on_Ignore();
    display_off_Ignore();
    sensors_read_uncalibrated_StubWithCallback(sensors_read);
    memory_write_data_Ignore();
    memory_erase_page_Ignore();
    timeout_reset_Ignore();
    ui_yes_no_IgnoreAndReturn(true);
    get_clicks_IgnoreAndReturn(NONE);
    get_single_click_or_throw_Ignore();
}

void test_calibrate_sensors(void) {
    calibrate_sensors(0);
}

