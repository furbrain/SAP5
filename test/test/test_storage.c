#include "unity.h"
#include "CException.h"
#include "storage.h"
#include "mock_memory.h"
#include "mock_utils.h"
#include "mem_locations.h"
#include <stdio.h>
#include <string.h>

int target_array[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

extern uint8_t config_space[];
extern uint8_t leg_space[];

struct CONFIG test_config = {
    {{1,0,2},{0,1,5}},                   //axis orientation
    { //calib section
        {{1.0,0,0,0},{0,1.0,0,0},{0,0,1.0,0}}, //accel matrix
        {{1.0,0,0,0},{0,1.0,0,0},{0,0,1.0,0}}, //mag matrix
        0.090                              //laser offset
    },
    POLAR,                               //Polar display style
    METRIC,                              //metric units
    30                                   //30s timeout
};

    

int write_dword_replacement(void* ptr, const int* src, int num_calls) {
    int i;
    if ((size_t)ptr % 8) return -1;
    if (((int*)ptr < target_array) || ((int*)ptr > target_array+4)) 
        if (((uint8_t*)ptr < config_space) || ((uint8_t*)ptr > config_space+APP_CONFIG_SIZE)) 
            Throw(0xBADADDA);
    for (i=0; i<8; i++) {
        if (*((uint8_t*)ptr+i) != 0xff) Throw(0xBADDA7A);
    }
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


