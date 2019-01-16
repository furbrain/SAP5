#include "unity.h"
#include "survey.h"
#include "exception.h"
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
    survey_current_init();
    TEST_ASSERT_EQUAL(1, survey_current.number);
    TEST_ASSERT_EQUAL(1, survey_current.max_station);
    //last leg should be backward so that next leg will be forward if toggling
    TEST_ASSERT_EQUAL(false, survey_current.last_leg_forward);
    TEST_ASSERT_EQUAL(EIGHT_HOURS, survey_current.start_time);
}

void leg_get_survey_details_cb(int survey, int *max_station, time_t *first_leg, bool *last_leg_forward, int num_calls) {
    if (survey != 3) THROW_WITH_REASON("Survey not found", ERROR_SURVEY_NOT_FOUND);
    *max_station = 5;
    *first_leg = 9;
    *last_leg_forward = false;
}

void test_survey_current_init_with_recent_survey(void)
{
    leg_find_last_ExpectAndReturn(&test_leg);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS);
    leg_get_survey_details_StubWithCallback(leg_get_survey_details_cb);
    survey_current_init();    
    TEST_ASSERT_EQUAL(3, survey_current.number);
    TEST_ASSERT_EQUAL(5, survey_current.max_station);
    TEST_ASSERT_EQUAL(9, survey_current.start_time);
    TEST_ASSERT_EQUAL(false, survey_current.last_leg_forward);
}

void test_survey_current_init_with_old_survey(void) {
    leg_find_last_ExpectAndReturn(&test_leg);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS+14);
    utils_get_time_ExpectAndReturn(EIGHT_HOURS+14);
    survey_current_init();    
    TEST_ASSERT_EQUAL(4, survey_current.number);
    TEST_ASSERT_EQUAL(1, survey_current.max_station);
    TEST_ASSERT_EQUAL(EIGHT_HOURS+14, survey_current.start_time);
    TEST_ASSERT_EQUAL(false, survey_current.last_leg_forward);
}

void test_survey_populate(void) {
    struct SURVEY survey;
    leg_get_survey_details_StubWithCallback(leg_get_survey_details_cb);
    survey_populate(&survey, 3);
    TEST_ASSERT_EQUAL(3, survey.number);
    TEST_ASSERT_EQUAL(5, survey.max_station);
    TEST_ASSERT_EQUAL(9, survey.start_time);
}

void test_survey_start_new(void) {
    utils_get_time_ExpectAndReturn(EIGHT_HOURS);
    survey_current.number = 12;
    survey_current.max_station = 15;
    survey_current.start_time = 31;
    survey_current.last_leg_forward = false;
    survey_start_new();
    TEST_ASSERT_EQUAL(13, survey_current.number);
    TEST_ASSERT_EQUAL(1, survey_current.max_station);
    TEST_ASSERT_EQUAL(false, survey_current.last_leg_forward);
    TEST_ASSERT_EQUAL(EIGHT_HOURS, survey_current.start_time);    
}

void test_survey_add_leg_boring(void) {
    struct SURVEY survey = {2, 10, true, 12};
    survey_add_leg(&survey, &test_leg);
    TEST_ASSERT_EQUAL(2, survey.number);
    TEST_ASSERT_EQUAL(10, survey.max_station);
    TEST_ASSERT_EQUAL(true, survey.last_leg_forward);
    TEST_ASSERT_EQUAL(12, survey.start_time);    
}

void test_survey_add_leg_increases_max_station_with_to(void) {
    struct SURVEY survey = {2, 3, true, 12};
    survey_add_leg(&survey, &test_leg);
    TEST_ASSERT_EQUAL(2, survey.number);
    TEST_ASSERT_EQUAL(5, survey.max_station);
    TEST_ASSERT_EQUAL(true, survey.last_leg_forward);
    TEST_ASSERT_EQUAL(12, survey.start_time);    
}

void test_survey_add_leg_increases_max_station_with_from(void) {
    struct SURVEY survey = {2, 2, true, 12};
    struct LEG leg = {12, 3, 6, 3, {1.0, 1.0, 1.0}};
    survey_add_leg(&survey, &leg);
    TEST_ASSERT_EQUAL(2, survey.number);
    TEST_ASSERT_EQUAL(6, survey.max_station);
    //note this is false because this is now the latest leg - and it was backwards...
    TEST_ASSERT_EQUAL(false, survey.last_leg_forward); 
    TEST_ASSERT_EQUAL(12, survey.start_time);    
}

