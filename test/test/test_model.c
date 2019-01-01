#include <string.h>
#include "unity.h"
#include "model.h"
#include "leg.h"
#include "exception.h"
#include "mock_memory.h"

struct LEG test_leg_array[6] = {
    {13, 3, 1, 2, {1.0, 2.0, 3.0}},
    {14, 3, 4, 2, {1.0, 2.0, 3.0}},
    {15, 1, 1, 2, {1.0, 2.0, 3.0}},
    {16, 2, 2, 3, {1.0, 2.0, 3.0}},
    {17, 2, 1, 3, {1.0, 2.0, 3.0}},
    {12, 3, 2, 3, {1.0, 2.0, 3.0}},
};

void write_data_replacement(const void* ptr, const void* src, int length, int num_calls) {
    int i;
    if ((size_t)ptr % 8)
        THROW_WITH_REASON("Destination ptr not on dword boundary", ERROR_FLASH_STORE_FAILED);
    if (((uint8_t*)ptr < leg_store.raw) || (((uint8_t*)ptr)+length > leg_store.raw+APP_LEG_SIZE)) 
        THROW_WITH_REASON("Destination memory out of range", ERROR_FLASH_STORE_FAILED);
    for (i=0; i<length; i++) {
        if (*((uint8_t*)ptr+i) != 0xff) {
            THROW_WITH_REASON("Memory not been cleared for write", ERROR_FLASH_STORE_FAILED);
        }
    }
    memcpy((void *)ptr, src, length);
}

void erase_page_replacement(const void *ptr, int num_calls) {
    if ((size_t)ptr % 0x800) 
        THROW_WITH_REASON("Erase page not on page boundary", ERROR_FLASH_STORE_FAILED);
    memset((void*)ptr, 0xff, 0x800);
}


void setUp(void)
{
    erase_page_StubWithCallback(erase_page_replacement);
    memset(leg_store.raw, 0xff, APP_LEG_SIZE);
}

void tearDown(void)
{
}


void add_test_legs(void) {
    int i;
    write_data_StubWithCallback(write_data_replacement);
    for (i=0; i<6; i++) {
        leg_save(&test_leg_array[i]);
    }
}

/* generate a model of the survey given by survey*/
void test_model_generate(void) {
    TEST_IGNORE_MESSAGE("design test for model_generate");
}

/*reset all lists*/
void test_reset_lists(void) {
    model_leg_count = 1;
    processed_leg_count = 2;
    station_count = 3;
    reset_lists();
    TEST_ASSERT_EQUAL(0, model_leg_count);
    TEST_ASSERT_EQUAL(0, processed_leg_count);
    TEST_ASSERT_EQUAL(0, station_count);
}

/*set first from station in survey to (0,0,0)
  Throws ERROR_SURVEY_PROCESS_FAIL if no legs found*/
void test_initialise_first_station(void) {
    add_test_legs();
    initialise_first_station(2);
    TEST_ASSERT_EQUAL(2,model_stations[0].number);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[0]);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[1]);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[2]);
    TEST_ASSERT_THROWS(initialise_first_station(5),ERROR_NO_SURVEY_DATA);
}

/*if a station has already been found, return a pointer to it
 * otherwise return null*/
void test_find_station(void) {
    TEST_IGNORE_MESSAGE("Need to Implement test");    
}
/* add a station to the found list, along with its position*/
void test_add_station(void) {
    TEST_IGNORE_MESSAGE("Need to Implement test");
}

/* add a model leg to the list*/
void test_add_leg(void) {
    TEST_IGNORE_MESSAGE("Need to Implement test");    
}

/*test whether a leg has been processed*/
void test_leg_has_been_processed(void) {
    int i,j;
    reset_lists();
    for (i=0; i<6; ++i) {
        for (j=0; j<i; ++j) {
            TEST_ASSERT_TRUE(leg_has_been_processed(&test_leg_array[j]));
        }
        for (j=i; j<6; ++j) {
            TEST_ASSERT_FALSE(leg_has_been_processed(&test_leg_array[j]));
        }
        mark_leg_as_processed(&test_leg_array[i]);
    }
}

/*mark a survey leg as having been processed*/
void test_mark_leg_as_processed_throws_error(void) {
    int i;
    reset_lists();
    for (i=0; i<MODEL_MAX_STORAGE; ++i) {
        mark_leg_as_processed(test_leg_array);
    }
    TEST_ASSERT_THROWS(mark_leg_as_processed(test_leg_array), ERROR_SURVEY_TOO_BIG);
}

/*with arrays of double[3], add offset to origin and store the answer in result*/
void test_add_delta(void) {
    struct test_field {
        double a[3];
        double b[3];
        double result[3];
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{1, 0, 2}, {2, 1, -2}, {3, 1, 0}},
        {{1.5, 2.5, 3.3}, {1.0, 2.5, 3.2}, {2.5, 5.0, 6.5}}
    };
    int i;
    double result[3];
    for (i=0; i<3; ++i) {
        add_delta(test_cases[i].a, test_cases[i].b, result);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY(test_cases[i].result, result, 3);
    }
}

/*with arrays of double[3], subtract offset from origin and store the answer in result*/
void test_sub_delta(void) {
    struct test_field {
        double result[3];
        double b[3];
        double a[3];
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{1, 0, 2}, {2, 1, -2}, {3, 1, 0}},
        {{1.5, 2.5, 3.3}, {1.0, 2.5, 3.2}, {2.5, 5.0, 6.5}}
    };
    int i;
    double result[3];
    for (i=0; i<3; ++i) {
        sub_delta(test_cases[i].a, test_cases[i].b, result);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY(test_cases[i].result, result, 3);
    }
}
