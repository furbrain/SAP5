#include <string.h>
#include "unity.h"
#include "model.h"
#include "leg.h"
#include "exception.h"
#include "mock_memory.h"

struct LEG test_leg_array[] = {
    {13, 3, 1, 2, {1.0, 2.0, 3.0}},
    {14, 3, 4, 2, {1.0, 2.0, 3.0}},
    {15, 1, 1, 2, {1.0, 2.0, 3.0}},
    {16, 2, 2, 3, {1.0, 2.0, 3.0}},
    {17, 2, 1, 3, {1.0, 2.0, 3.0}},
    {18, 4, 1, 2, {1.0, 0.0, 0.0}}, //1={0,0,0}, 2={1,0,0}
    {19, 4, 3, 2, {0.0, -2.0, 0.0}}, //3={1,2,0}
    {20, 4, 4, 5, {1.0, 1.0, 1.0}}, //delayed leg - needs connecting (5={2,3,2})
    {21, 4, 3, 4, {0.0, 0.0, 1.0}}, //4={1,2,1}
    {22, 4, 4, 1, {0.0, -2.0, 0.0}}, //1* = {1,0,1}
    {23, 5, 1, 2, {0.0, 1.0, 2.0}},
    {24, 5, 3, 4, {0.0, 1.0, 2.0}},
    {25, 6, 1, LEG_SPLAY, {0.0, 1.0, 0.0}},
    {25, 6, 1, LEG_SPLAY, {0.0, -1.0, 0.0}},
    {12, 3, 2, 3, {1.0, 2.0, 3.0}},
};

int test_leg_count = sizeof(test_leg_array) / sizeof(test_leg_array[0]);

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

void add_test_legs(void) {
    int i;
    memory_write_data_StubWithCallback(write_data_replacement);
    for (i=0; i<test_leg_count; i++) {
        leg_save(&test_leg_array[i]);
    }
}

void setUp(void)
{
    memory_erase_page_StubWithCallback(erase_page_replacement);
    memset(leg_store.raw, 0xff, APP_LEG_SIZE);
    reset_lists();
    add_test_legs();
}

void tearDown(void)
{
}

/* generate a model of the survey given by survey*/
void test_model_generate(void) {
    /* test with a circular survey with one delayed leg connection*/
    struct test_field {
        uint8_t number;
        double pos[3];
    };
    struct test_field test_cases[6] = {
        {1, {0,0,0}},
        {2, {1,0,0}},
        {3, {1,2,0}},
        {4, {1,2,1}},
        {5, {2,3,2}},
        {1, {1,0,1}} //this last one is a "fake" station, so not searched in main loop
    };
    int i;
    struct MODEL_CAVE cave;
    const struct MODEL_STATION *station;
    char text[20];
    model_generate(4, &cave);
    TEST_ASSERT_EQUAL(6, cave.leg_count);
    TEST_ASSERT_EQUAL(6, cave.station_count);
    for (i=0; i<5; ++i) {
        station = find_station(test_cases[i].number);
        sprintf(text, "Station: %d", test_cases[i].number);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(test_cases[i].pos, station->pos, 3, text);
    }
    // find the "fake leg"
    for (i=0; i< cave.leg_count; ++i) {
        struct MODEL_LEG *leg = &cave.legs[i];
        if (leg->from->number==leg->to->number) {
            TEST_ASSERT_EQUAL_DOUBLE_ARRAY(test_cases[5].pos, leg->to->pos, 3);
        }
    }
}

void test_model_generate_fails_with_disjoint_survey(void) {
    struct MODEL_CAVE cave;
    TEST_ASSERT_THROWS(model_generate(5, &cave), ERROR_SURVEY_IS_DISJOINT);
}

void test_model_generate_with_splay_legs(void) {
    struct MODEL_CAVE cave;
    model_generate(6, &cave);
    TEST_ASSERT_EQUAL(2, cave.leg_count);
    TEST_ASSERT_EQUAL(3, cave.station_count);
}

/*reset all lists*/
void test_reset_lists(void) {
    model_leg_count = 1;
    processed_leg_count = 2;
    station_count = 3;
    TEST_ASSERT_EQUAL(1, model_leg_count);
    TEST_ASSERT_EQUAL(2, processed_leg_count);
    TEST_ASSERT_EQUAL(3, station_count);
    reset_lists();
    TEST_ASSERT_EQUAL(0, model_leg_count);
    TEST_ASSERT_EQUAL(0, processed_leg_count);
    TEST_ASSERT_EQUAL(0, station_count);
}

/*set first from station in survey to (0,0,0)
  Throws ERROR_SURVEY_PROCESS_FAIL if no legs found*/
void test_initialise_first_station(void) {
    initialise_first_station(2);
    TEST_ASSERT_EQUAL(2,model_stations[0].number);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[0]);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[1]);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, model_stations[0].pos[2]);
    TEST_ASSERT_THROWS(initialise_first_station(7),ERROR_NO_SURVEY_DATA);
}

/* add a station to the found list, along with its position*/
void test_add_station(void) {
    struct MODEL_STATION *station;
    TEST_ASSERT_EQUAL(0, station_count);
    station = add_station(1,(double[3]) {1,2,3});
    TEST_ASSERT_EQUAL(1, station_count);
    TEST_ASSERT_EQUAL_PTR(station, &model_stations[0]);
    TEST_ASSERT_EQUAL(1, station->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, station->pos[0]);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, station->pos[1]);
    TEST_ASSERT_EQUAL_DOUBLE(3.0, station->pos[2]);
}

void test_add_station_throws_error_when_full(void) {
    int i;
    for (i=0; i< MODEL_MAX_STORAGE; ++i) {
        add_station(i,(double[3]) {1,2,3});
    }
    TEST_ASSERT_THROWS(add_station(i,(double[3]) {1,2,3}), ERROR_SURVEY_TOO_BIG);
}

/*if a station has already been found, return a pointer to it
 * otherwise return null*/
void test_find_station(void) {
    struct MODEL_STATION *a, *b, *c, *d, *e;
    a = add_station(2, (double[3]) {1,2,3});
    b = add_station(4, (double[3]) {1,2,3});
    c = add_station(3, (double[3]) {1,2,3});
    d = add_station(4, (double[3]) {1,2,3});
    e = add_station(LEG_SPLAY, (double[3]) {1,2,3});
    TEST_ASSERT_EQUAL_PTR(a, find_station(2));
    TEST_ASSERT_EQUAL_PTR(b, find_station(4));
    TEST_ASSERT_EQUAL_PTR(c, find_station(3));
    TEST_ASSERT_EQUAL_PTR(b, find_station(4));
    TEST_ASSERT_NULL(find_station(LEG_SPLAY))
    TEST_ASSERT_NULL(find_station(15));
}

/* add a model leg to the list*/
void test_add_leg(void) {
    struct MODEL_STATION *a, *b, *c, *d;
    a = add_station(2, (double[3]) {1,2,1});
    b = add_station(4, (double[3]) {1,1,2});
    c = add_station(3, (double[3]) {1,2,4});
    d = add_station(4, (double[3]) {1,2,5});
    add_leg(a,b);
    add_leg(b,c);
    add_leg(c,d);
    TEST_ASSERT_EQUAL_PTR(a,model_legs[0].from);
    TEST_ASSERT_EQUAL_PTR(b,model_legs[0].to);
    TEST_ASSERT_EQUAL_PTR(b,model_legs[1].from);
    TEST_ASSERT_EQUAL_PTR(c,model_legs[1].to);
    TEST_ASSERT_EQUAL_PTR(c,model_legs[2].from);
    TEST_ASSERT_EQUAL_PTR(d,model_legs[2].to);
}

void test_add_leg_throw_error_when_full(void) {
    int i;
    for (i=0; i < MODEL_MAX_STORAGE; ++i) {
        add_leg(NULL,NULL);
    }
    TEST_ASSERT_THROWS(add_leg(NULL, NULL), ERROR_SURVEY_TOO_BIG);
}

/*test whether a leg has been processed*/
void test_leg_has_been_processed(void) {
    int i,j;
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
