#include <string.h>

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
    {14, 3, 3, 2, {1.0, 2.0, 3.0}},
    {15, 1, 1, 2, {1.0, 2.0, 3.0}},
    {16, 2, 2, 3, {1.0, 2.0, 3.0}},
    {17, 2, 1, 3, {1.0, 2.0, 3.0}},
    {12, 3, 2, 3, {1.0, 2.0, 3.0}},
};

void write_data_replacement(void* ptr, const void* src, int length, int num_calls) {
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
    memcpy(ptr, src, length);
}

void erase_page_replacement(void *ptr, int num_calls) {
    if ((size_t)ptr % 0x800) 
        THROW_WITH_REASON("Erase page not on page boundary", ERROR_FLASH_STORE_FAILED);
    memset(ptr, 0xff, 0x800);
}


void setUp(void)
{
    write_data_StubWithCallback(write_data_replacement);
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

void add_test_legs(void) {
    int i;
    for (i=0; i<6; i++) {
        leg_save(&test_leg_array[i]);
    }
}
   

void test_leg_find(void) {
    struct LEG *found_leg;
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
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[0], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[1].tm);
}

void test_leg_save_double(void) {
    leg_save(&test_leg);
    leg_save(&test_leg);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, &leg_store.legs[1], sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT32(0xffffffff, leg_store.legs[2].tm);
}

void test_leg_save_overflow(void) {
    int counter;
    struct LEG new_leg;
    TEST_IGNORE_MESSAGE("disabled for speed - reenable me!");
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
    TEST_IGNORE_MESSAGE("disabled for speed - reenable me!");
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

