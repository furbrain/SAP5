#include <string.h>
#include <stdlib.h>

#include "unity.h"
#include "leg.h"
#include "mock_memory.h"
#include "mem_locations.h"
#include "exception.h"

struct LEG test_leg = {
    12, //datetime
    0, //survey
    1, //from
    2, //to
    {1.0, 1.0, 2.0} //delta
};

struct LEG test_leg_array[6] = {
    {13, 3, 1, 2, {1.0, 2.0, 3.0}},
    {14, 3, 4, 2, {1.0, 2.0, 3.0}},
    {15, 1, 1, 2, {1.0, 2.0, 3.0}},
    {16, 2, 2, 3, {1.0, 2.0, 3.0}},
    {17, 2, 1, 3, {1.0, 2.0, 3.0}},
    {12, 3, 2, 3, {1.0, 2.0, 3.0}},
};

void memory_write_data_replacement(const void* ptr, const void* src, int length, int num_calls) {
    int i;
    if ((size_t)ptr % 8) {
        //printf("boundary error");
        THROW_WITH_REASON("Destination ptr not on dword boundary", ERROR_FLASH_STORE_FAILED);
    }
    if (((uint8_t*)ptr < leg_store.raw) || (((uint8_t*)ptr)+length > leg_store.raw+APP_LEG_SIZE)) {
        //printf("out of range");
        THROW_WITH_REASON("Destination memory out of range", ERROR_FLASH_STORE_FAILED);
    }
    for (i=0; i<length; i++) {
        if (*((uint8_t*)ptr+i) != 0xff) {
            //printf("memory not cleared");
            THROW_WITH_REASON("Memory not been cleared for write", ERROR_FLASH_STORE_FAILED);
        }
    }
    //printf("write: %hx\n", (uint16_t)ptr);
    memcpy((void *)ptr, src, length);
}

void memory_erase_page_replacement(const void *ptr, int num_calls) {
    if ((size_t)ptr % 0x800) 
        THROW_WITH_REASON("Erase page not on page boundary", ERROR_FLASH_STORE_FAILED);
    memset((void*)ptr, 0xff, 0x800);
}


void setUp(void)
{
    memory_erase_page_StubWithCallback(memory_erase_page_replacement);
    memset(leg_store.raw, 0xff, APP_LEG_SIZE);
}

void tearDown(void)
{
}

void test_leg_create(void) {
    struct LEG leg;
    double data[3] = {1.2,3.4,5.6};
    gsl_vector_view vector = gsl_vector_view_array(data, 3);
    leg = leg_create(10,1,2,3,&vector.vector);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL_FLOAT(1.2,leg.delta[0]);
    TEST_ASSERT_EQUAL_FLOAT(3.4,leg.delta[1]);
    TEST_ASSERT_EQUAL_FLOAT(5.6,leg.delta[2]);
}

void add_test_legs(void) {
    int i;
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    for (i=0; i<6; i++) {
        leg_save(&test_leg_array[i]);
    }
}
   

void test_leg_find(void) {
    const struct LEG *found_leg;
    add_test_legs();
    found_leg = leg_find(1, 0);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&leg_store.legs[2], found_leg, "1,0->2");

    found_leg = leg_find(1,1);
    TEST_ASSERT_NULL_MESSAGE(found_leg, "1,1->NULL");
    
    found_leg = leg_find(2, 1);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&leg_store.legs[4], found_leg, "2,1 -> 4");
    
    found_leg = leg_find(3,0);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&leg_store.legs[5], found_leg, "3, 0 -> 5");
    
    found_leg = leg_find(3,2);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&leg_store.legs[1], found_leg, "3,2 -> 1");

    found_leg = leg_find(3,3);
    TEST_ASSERT_NULL_MESSAGE(found_leg, "3,3->NULL");
}


void test_leg_find_last(void) {
    const struct LEG *found_leg;
    add_test_legs();
    found_leg = leg_find_last();
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&leg_store.legs[1], found_leg, "should be last one in survey 3");
}

void test_leg_get_survey_details(void) {
    struct test_field {
        int survey;
        int max_station;
        time_t first_time;
        bool forward;
    };
    struct test_field test_cases[3] = {
        {1, 2, 15, true},
        {2, 3, 16, true},
        {3, 4, 12, false}
    };
    int i, max_station;
    time_t first_time;
    bool forward;
    add_test_legs();
    for (i = 0; i<3; i++) {
        leg_get_survey_details(test_cases[i].survey, &max_station, &first_time, &forward);
        TEST_ASSERT_EQUAL(test_cases[i].max_station, max_station);   
        TEST_ASSERT_EQUAL(test_cases[i].first_time, first_time);   
        TEST_ASSERT_EQUAL(test_cases[i].forward, forward);   
    }
    TEST_ASSERT_THROWS(leg_get_survey_details(34, &max_station, &first_time, &forward), ERROR_SURVEY_NOT_FOUND);
}

void test_leg_find_last_if_no_legs(void) {
    const struct LEG *found_leg;
    found_leg = leg_find_last();
    TEST_ASSERT_NULL_MESSAGE(found_leg, "leg_find_last should return NULL if no legs");
}



void test_leg_spans_boundary(void) {
    void *leg_start = &leg_store;
    void *leg_end = &leg_store.legs[MAX_LEG_COUNT];
    struct test_field {
        void  *result;
        struct LEG *leg;
    };
    struct test_field test_cases[11] = {
        {NULL, NULL},
        {leg_start, leg_start},
        {NULL, leg_start+0x04},
        {NULL, leg_start+0x800},
        {NULL, leg_end-0x02},
        {NULL, leg_end},
        {leg_start+0x800, leg_start+0x7E0},
        {leg_start+0x800, leg_start+0x7FE},
        {leg_start+0x1000, leg_start+0xFE0},
        {leg_start+0x1000, leg_start+0xFFE},
        {(void*)0x9D002800, (struct LEG*)(0x9D0027FF)}
    };
    int i;
    for (i=0; i<11; i++) {
        TEST_ASSERT_EQUAL_PTR(test_cases[i].result, leg_spans_boundary(test_cases[i].leg));
    }
}


void test_leg_save_single(void) {
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[0], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[1].tm);
}

void test_leg_save_double(void) {
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    leg_save(&test_leg);
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[1], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[2].tm);
}

void test_leg_save_overflow(void) {
    int counter;
    struct LEG new_leg;
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    counter = APP_LEG_SIZE/sizeof(new_leg);
    while(counter--) {
        leg_save(&new_leg);
        new_leg.tm++;
    }
    leg_save(&new_leg);
    TEST_ASSERT_EQUAL_MEMORY(&new_leg, &leg_store.legs[0], sizeof(new_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[1].tm);
}

void test_leg_save_with_tm_lsb_is_0xff(void) {
    struct LEG new_leg;
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    new_leg.tm = 0xff;
    leg_save(&new_leg);
    new_leg.tm = 0x100;
    leg_save(&new_leg);
    TEST_ASSERT_EQUAL(0xff,leg_store.legs[0].tm);
    TEST_ASSERT_EQUAL(0x100,leg_store.legs[1].tm);
}

void test_leg_save_page_overflow(void) {
    /* check works correctly when leg boundary aligns with page boundary -- and when it doesn't */
    int counter;
    struct LEG new_leg;
    int index = 0x800/sizeof(new_leg);
    memory_write_data_StubWithCallback(memory_write_data_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    // fill whole leg space
    counter = 0;
    while(counter < APP_LEG_SIZE/sizeof(new_leg)) {
        leg_save(&new_leg);
        new_leg.tm++;
        counter++;
    }
    //fill first page
    counter  = index;
    while(counter--) {
        leg_save(&new_leg);
        new_leg.tm++;
    }
    leg_save(&new_leg);
    TEST_ASSERT_EQUAL_MEMORY(&new_leg, &leg_store.legs[index], sizeof(new_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[index+1].tm);
}

void test_leg_save_fails(void) {
    memory_write_data_ExpectAndThrow(NULL, NULL, sizeof(struct LEG), ERROR_FLASH_STORE_FAILED);
    memory_write_data_IgnoreArg_ptr();
    memory_write_data_IgnoreArg_src();
    TEST_ASSERT_THROWS(leg_save(&test_leg), ERROR_FLASH_STORE_FAILED);
}

/* convert a pair of stations to text, do not alter the returned string - owned by this module */
void test_stations_to_text(void) {
    struct test_field {
        uint8_t from;
        uint8_t to;
        const char *text;
    };
    struct test_field test_cases[5] = {
        {1, 2, "1  ->  2"},
        {2, 4, "2  ->  4"},
        {3, LEG_SPLAY, "3  ->  -"},
        {LEG_SPLAY, 5, "-  ->  5"},
        {200, 201, "200  ->  201"}
    };
    const char *text;
    int i;
    for (i=0; i<5; i++) {
        text = leg_stations_to_text(test_cases[i].from, test_cases[i].to);
        TEST_ASSERT_EQUAL_STRING(test_cases[i].text, text);
    }
}


/* test encoding end-to-end */
void test_stations_encode_decode(void) {
    int i;
    uint8_t from, to, rfrom, rto;
    int32_t intermediate;
    for (i=0; i<100; i++) {
        from = rand() % 256;
        to = rand() %256;
        intermediate = leg_stations_encode(from, to);
        leg_stations_decode(intermediate, &rfrom, &rto);
        TEST_ASSERT_EQUAL(from, rfrom);
        TEST_ASSERT_EQUAL(to, rto); 
    }
}

void test_leg_is_splay(void) {
    struct test_field{
        struct LEG leg;
        bool result;
    };
    
    struct test_field test_cases[] = {
        {{13, 2, LEG_SPLAY, 2, {1.0, 2.0, 3.0}},true},
        {{14, 2, 4, LEG_SPLAY,  {1.0, 2.0, 3.0}},true},
        {{15, 1, 1, 2, {1.0, 2.0, 3.0}},false}
    };
    int i;
    for (i=0; i<3; i++) {
        TEST_ASSERT_EQUAL(test_cases[i].result,leg_is_splay(&test_cases[i].leg));
    }
}

void test_leg_first_normal(void) {
    leg_store.legs[0] = test_leg;
    TEST_ASSERT_EQUAL_PTR(&leg_store.legs[0], leg_first());
}

void test_leg_first_first_not_valid(void) {
    leg_store.legs[0x20] = test_leg;
    TEST_ASSERT_EQUAL_PTR(&leg_store.legs[0x20], leg_first());

}

void test_leg_first_none_valid(void) {
    TEST_ASSERT_EQUAL_PTR(NULL, leg_first());
}

void test_leg_enumerate_first(void) {
    leg_store.legs[0] = test_leg;
    leg_store.legs[3] = test_leg;
    const struct LEG *leg = NULL;
    TEST_ASSERT_EQUAL_PTR(&leg_store.legs[0], leg_enumerate(leg));
}


void test_leg_enumerate_normal(void) {
    leg_store.legs[2] = test_leg;
    leg_store.legs[3] = test_leg;
    const struct LEG *leg = &leg_store.legs[2];
    TEST_ASSERT_EQUAL_PTR(&leg_store.legs[3], leg_enumerate(leg));
}

void test_leg_enumerate_skipped(void) {
    leg_store.legs[2] = test_leg;
    leg_store.legs[6] = test_leg;
    const struct LEG *leg = &leg_store.legs[2];
    TEST_ASSERT_EQUAL_PTR(&leg_store.legs[6], leg_enumerate(leg));
}

void test_leg_enumerate_none(void) {
    leg_store.legs[2] = test_leg;
    leg_store.legs[3] = test_leg;
    const struct LEG *leg = &leg_store.legs[3];
    TEST_ASSERT_EQUAL_PTR(NULL, leg_enumerate(leg));
}
