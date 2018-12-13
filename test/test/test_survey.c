#include "unity.h"
#include "survey.h"
#include "mock_leg.h"
#include "mock_utils.h"

struct LEG test_leg = {
    12, //datetime
    3, //survey
    3, //from
    5, //to
    {1.0, 1.0, 2.0} //delta
};


void setUp(void)
{
}

void tearDown(void)
{
}

void test_survey_init_first_time(void)
{
    leg_find_last_ExpectAndReturn(NULL);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS);
    survey_init();
    TEST_ASSERT_EQUAL(1, survey_current.number);
    TEST_ASSERT_EQUAL(1, survey_current.max_station);
    TEST_ASSERT_EQUAL(true, survey_current.forward);
    TEST_ASSERT_EQUAL(EIGHT_HOURS, survey_current.start_time);
}

void leg_get_survey_details_cb(int survey, int *max_station, time_t *first_leg, int num_calls) {
    *max_station = 5;
    *first_leg = 3;
}

void test_survey_init_with_recent_survey(void)
{
    leg_find_last_ExpectAndReturn(&test_leg);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS);
    leg_get_survey_details_StubWithCallback(leg_get_survey_details_cb);
    survey_init();    
    TEST_ASSERT_EQUAL(3, survey_current.number);
    TEST_ASSERT_EQUAL(5, survey_current.max_station);
    TEST_ASSERT_EQUAL(true, survey_current.forward);
}

void test_survey_init_with_old_survey(void) {
    leg_find_last_ExpectAndReturn(&test_leg);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS+14);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS+14);
    survey_init();    
    TEST_ASSERT_EQUAL(4, survey_current.number);
    TEST_ASSERT_EQUAL(1, survey_current.max_station);
    TEST_ASSERT_EQUAL(true, survey_current.forward);
}

