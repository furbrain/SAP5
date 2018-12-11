#include <string.h>

#include "unity.h"
#include "leg.h"
#include "mock_memory.h"
#include "mock_storage.h"
#include "mem_locations.h"
#include "exception.h"

struct LEG test_leg = {
    12, //datetime
    0, //survey
    1, //from
    2, //to
    {1.0, 1.0, 2.0} //delta
};

int write_data_replacement(void* ptr, const void* src, int length, int num_calls) {
    int i;
    if ((size_t)ptr % 8) return -1;
    if (((uint8_t*)ptr < leg_store.raw) || ((uint8_t*)ptr > leg_store.raw+APP_LEG_SIZE)) 
        Throw(0xBADADDA);
    for (i=0; i<length; i++) {
        if (*((uint8_t*)ptr+i) != 0xff) {
            Throw(0xBADDA7A);
        }
    }
    memcpy(ptr, src, length);
    return 0;
}

int erase_page_replacement(void *ptr, int num_calls) {
    if ((size_t)ptr % 0x800) return -1;
    memset(ptr, 0xff, 0x800);
    return 0;
}


void setUp(void)
{
    erase_page_StubWithCallback(erase_page_replacement);
    memset(leg_store.raw, 0xff, APP_LEG_SIZE);
}

void tearDown(void)
{
}

void test_leg_create(void) {
    struct LEG leg;
    leg = leg_create(10,1,2,3,(double[3]){1.2,3.4,5.6});
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL(10,leg.tm);
    TEST_ASSERT_EQUAL_FLOAT(1.2,leg.delta[0]);
}

void test_leg_find(void) {
    TEST_IGNORE_MESSAGE("Need to Implement leg_find");
}

void test_leg_find_last(void) {
    TEST_IGNORE_MESSAGE("Need to Implement leg_find_last");
}

void test_leg_spans_boundary(void) {
    struct test_field {
        void  *result;
        struct LEG *leg;
    };
    struct test_field test_cases[6] = {
        {NULL, NULL},
        {(void*)0xA0002000, (struct LEG*)0xA0002000},
        {NULL, (struct LEG*)0xA0002004},
        {NULL, (struct LEG*)(0xA0002000 - sizeof(struct LEG))},
        {(void*)0xA0002000, (struct LEG*)(0xA0002000-2)},
        {(void*)0xA0002800, (struct LEG*)(0xA00027FF)}
    };
    int i;
    for (i=0; i<6; i++) {
        TEST_ASSERT_EQUAL_PTR(test_cases[i].result, leg_spans_boundary(test_cases[i].leg));
    }
}


void test_leg_save_single(void) {
    write_data_StubWithCallback(write_data_replacement);
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[0], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[1].tm);
}

void test_leg_save_double(void) {
    write_data_StubWithCallback(write_data_replacement);
    leg_save(&test_leg);
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[1], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[2].tm);
}

void test_leg_save_overflow(void) {
    int counter;
    struct LEG new_leg;
    write_data_StubWithCallback(write_data_replacement);
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
    struct LEG *leg_ptr;
    write_data_StubWithCallback(write_data_replacement);
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
    write_data_StubWithCallback(write_data_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    // fill whole leg space
    counter = 0;
    while(counter < MAX_LEG_COUNT) {
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

