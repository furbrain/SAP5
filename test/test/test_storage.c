#include "unity.h"
#include "CException.h"
#include "storage.h"
#include "mock_memory.h"
#include "utils.h"
#include "mem_locations.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/tmr1.h"
#include <stdio.h>
#include <string.h>
#include <xc.h>

int target_array[4] = {1,2,3,4};

extern uint8_t config_space[];
extern uint8_t leg_space[];

struct CONFIG test_config = {
    {{1,0,2},{0,1,5}},                   //axis orientation
    { //calib section
        {{1.0k,0,0,0},{0,1.0k,0,0},{0,0,1.0k,0}}, //accel matrix
        {{1.0k,0,0,0},{0,1.0k,0,0},{0,0,1.0k,0}}, //mag matrix
        0.090k                              //laser offset
    },
    POLAR,                               //Polar display style
    METRIC,                              //metric units
    30                                   //30s timeout
};

struct LEG test_leg = {
    12, //datetime
    0, //survey
    1, //from
    2, //to
    {1.0k, 1.0k, 2.0k} //delta
};
    

int write_dword_replacement(void* ptr, const int* src, int num_calls) {
    if ((size_t)ptr % 8) return -1;
    if (((int*)ptr < target_array) || ((int*)ptr > target_array+4)) 
        if (((uint8_t*)ptr < config_space) || ((uint8_t*)ptr > config_space+APP_CONFIG_SIZE)) 
            if (((uint8_t*)ptr < leg_space) || ((uint8_t*)ptr > leg_space+APP_LEG_SIZE)) 
                Throw(0xBADADDA);
    memcpy(ptr, src, 8);
    return 0;
}

int erase_page_replacement(void *ptr, int num_calls) {
    if ((size_t)ptr % 2048) return -1;
    memset(ptr, 0xff, 2048);
    return 0;
}

void setUp(void) {
    erase_page_StubWithCallback(erase_page_replacement);
    memset(config_space, 0xff, APP_CONFIG_SIZE);
    memset(leg_space, 0xff, APP_LEG_SIZE);
}

void test_write_data(void) {
    int zeros[4] = {0,0,0,0};
    write_dword_ExpectAndReturn((void*)target_array, (void*)zeros,0);
    write_dword_ExpectAndReturn((void*)(target_array+2), (void*)(zeros+2),0);
    write_data((uint8_t*) target_array,(uint8_t *) zeros, sizeof(zeros));
}


void test_stubbed_write_data(void) {
    int zeros[4] = {0,0,0,0};
    write_dword_StubWithCallback(write_dword_replacement);
    write_data((uint8_t*) target_array,(uint8_t *) zeros, sizeof(zeros));
    TEST_ASSERT_EQUAL_INT32_ARRAY(zeros, target_array, 4);
}


void test_write_config_single(void) {
    int result;
    write_dword_StubWithCallback(write_dword_replacement);
    result = write_config(&test_config);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_MEMORY(&test_config, config_space, sizeof(test_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(config_space + sizeof(test_config)));
}

void test_write_config_double(void) {
    int result;
    write_dword_StubWithCallback(write_dword_replacement);
    result = write_config(&test_config);
    result = write_config(&test_config);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_MEMORY(&test_config, config_space+sizeof(test_config), sizeof(test_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(config_space + sizeof(test_config)*2));
}

void test_write_config_overflow(void) {
    int counter;
    struct CONFIG new_config;
    write_dword_StubWithCallback(write_dword_replacement);
    counter = APP_CONFIG_SIZE/sizeof(test_config);
    memcpy(&new_config,&test_config,sizeof(new_config));
    new_config.axes.accel[0]=5;
    while(counter--) {
        write_config(&test_config);
    }
    write_config(&new_config);
    TEST_ASSERT_EQUAL_MEMORY(&new_config, config_space, sizeof(new_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(config_space + sizeof(test_config)));
}

void test_write_and_read_first_config(void) {
    int counter;
    const struct CONFIG *current_config;
    write_dword_StubWithCallback(write_dword_replacement);
    write_config(&test_config);
    current_config = read_config();
    TEST_ASSERT_EQUAL_MEMORY(&test_config, current_config, sizeof(current_config));
}


void test_write_and_read_second_config(void) {
    int counter;
    struct CONFIG new_config;
    const struct CONFIG *current_config;
    write_dword_StubWithCallback(write_dword_replacement);
    memcpy(&new_config,&test_config,sizeof(new_config));
    new_config.axes.accel[0]=5;
    write_config(&test_config);
    write_config(&new_config);
    current_config = read_config();
    TEST_ASSERT_EQUAL_MEMORY(&new_config, current_config, sizeof(current_config));
}

void test_read_last_config(void) {
    int counter;
    struct CONFIG new_config;
    const struct CONFIG *current_config;
    write_dword_StubWithCallback(write_dword_replacement);
    counter = APP_CONFIG_SIZE/sizeof(test_config)-1;
    memcpy(&new_config,&test_config,sizeof(new_config));
    new_config.axes.accel[0]=5;
    while(counter--) {
        write_config(&test_config);
    }
    write_config(&new_config);
    current_config = read_config();
    TEST_ASSERT_EQUAL_MEMORY(&new_config, current_config, sizeof(current_config));
}

void test_leg_spans_boundary(void) {
    struct test_field {
        uint8_t  *result;
        struct LEG *leg;
    };
    struct test_field test_cases[6] = {
        {NULL, NULL},
        {NULL, (struct LEG*)0xA0002000},
        {NULL, (struct LEG*)0xA0002004},
        {NULL, (struct LEG*)(0xA0002000 - sizeof(struct LEG))},
        {(uint8_t*)0xA0002000, (struct LEG*)(0xA0002000-2)},
        {(uint8_t*)0xA0002800, (struct LEG*)(0xA00027FF)}
    };
    int i;
    for (i=0; i<6; i++) {
        TEST_ASSERT_EQUAL_PTR(test_cases[i].result, leg_spans_boundary(test_cases[i].leg));
    }
}


void test_write_leg_single(void) {
    int result;
    write_dword_StubWithCallback(write_dword_replacement);
    result = write_leg(&test_leg);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, leg_space, sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(leg_space + sizeof(test_leg)));
}

void test_write_leg_double(void) {
    int result;
    write_dword_StubWithCallback(write_dword_replacement);
    result = write_leg(&test_leg);
    result = write_leg(&test_leg);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_MEMORY(&test_leg, leg_space+sizeof(test_leg), sizeof(test_leg));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(leg_space + sizeof(test_leg)*2));
}

void test_write_leg_overflow(void) {
    int counter;
    struct LEG new_leg;
    write_dword_StubWithCallback(write_dword_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    counter = APP_LEG_SIZE/sizeof(new_leg);
    while(counter--) {
        write_leg(&new_leg);
        new_leg.dt++;
    }
    write_leg(&new_leg);
    TEST_ASSERT_EQUAL_MEMORY(&new_leg, leg_space, sizeof(new_leg));
    TEST_ASSERT_EQUAL_UINT8(0xff, *(leg_space + sizeof(test_leg)));
}

void test_write_leg_with_dt_lsb_is_0xff(void) {
    struct LEG new_leg;
    struct LEG *leg_ptr;
    write_dword_StubWithCallback(write_dword_replacement);
    leg_ptr = (struct LEG*)leg_space;
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    new_leg.dt = 0xff;
    write_leg(&new_leg);
    new_leg.dt = 0x100;
    write_leg(&new_leg);
    TEST_ASSERT_EQUAL(0xff,leg_ptr[0].dt);
    TEST_ASSERT_EQUAL(0x100,leg_ptr[1].dt);
}

void test_write_leg_page_overflow(void) {
    /* check works correctly when leg boundary aligns with page boundary -- and when it doesn't */
    int counter;
    struct LEG new_leg;
    int index = 0x800/sizeof(new_leg);
    write_dword_StubWithCallback(write_dword_replacement);
    memcpy(&new_leg,&test_leg,sizeof(new_leg));
    // fill whole leg space
    counter = APP_LEG_SIZE/sizeof(new_leg);
    while(counter--) {
        write_leg(&new_leg);
        new_leg.dt++;
    }
    //fill first page
    counter  = index;
    while(counter--) {
        write_leg(&new_leg);
        new_leg.dt++;
    }
    write_leg(&new_leg);
    TEST_ASSERT_EQUAL_MEMORY(&new_leg, leg_space+sizeof(new_leg)*index, sizeof(new_leg));
    TEST_ASSERT_EQUAL_UINT8(0xff,leg_space[sizeof(new_leg) * (1+index)]);
}

