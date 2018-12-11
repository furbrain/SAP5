#include <string.h>

#include "unity.h"
#include "mock_memory.h"
#include "mock_storage.h"
#include "mem_locations.h"
#include "exception.h"
#include "config.h"
#include "mock_display.h"

extern const struct CONFIG default_config;

struct CONFIG test_config = {
    {{1,0,2},{0,1,5}},                   //axis orientation
    { //calib section
        {{1.0,0,0,0},{0,1.0,0,0},{0,0,1.0,0}}, //accel matrix
        {{1.0,0,0,0},{0,1.0,0,0},{0,0,1.0,0}}, //mag matrix
        0.090                              //laser offset
    },
    IMPERIAL,                               //Polar display style
    METRIC,                              //metric units
    30                                   //30s timeout
};



int write_data_replacement(void* ptr, const void* src, int length, int num_calls) {
    int i;
    if ((size_t)ptr % 8) return -1;
    if (((uint8_t*)ptr < config_store.raw) || (((uint8_t*)ptr)+length > config_store.raw+APP_CONFIG_SIZE)) 
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
    memset(&config_store.raw, 0xff, APP_CONFIG_SIZE);
    TEST_ASSERT_EACH_EQUAL_UINT32(0xffffffff, &config_store.raw[0], 0x200);
}

void tearDown(void)
{
}

void test_weird_memory(void) {
    //CEXCEPTION_T e;
    //struct CONFIG new_config;
    write_data_StubWithCallback(write_data_replacement);
    //new_config = test_config;
    config_save();
    //TEST_ASSERT_EQUAL_MEMORY(&test_config, &new_config, sizeof(new_config));
}

void test_config_save_single(void) {
    write_data_StubWithCallback(write_data_replacement);
    config = test_config;
    config_save();
    TEST_ASSERT_EQUAL_MEMORY(&test_config, &config_store.configs[0], sizeof(test_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, config_store.configs[1].axes.accel[0]);
}

void test_config_save_double(void) {
    write_data_StubWithCallback(write_data_replacement);
    config = test_config;
    config_save();
    config_save();
    TEST_ASSERT_EQUAL_MEMORY(&test_config, &config_store.configs[1], sizeof(test_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, config_store.configs[2].axes.accel[0]);
}

void test_config_save_overflow(void) {
    int counter;
    struct CONFIG new_config;
    write_data_StubWithCallback(write_data_replacement);
    counter = MAX_CONFIG_COUNT;
    new_config = test_config;
    config = test_config;
    new_config.axes.accel[0]=5;
    while(counter--) {
        config_save();
    }
    config = new_config;
    config_save();
    TEST_ASSERT_EQUAL_MEMORY(&new_config, &config_store.configs[0], sizeof(new_config));
    TEST_ASSERT_EQUAL_UINT8(0xff, config_store.configs[1].axes.accel[0]);
}

void test_config_read_with_no_data(void) {
    config = test_config;
    config_load();
    TEST_ASSERT_EQUAL_MEMORY(&default_config, &config, sizeof(config));
}    

void test_write_and_read_first_config(void) {
    write_data_StubWithCallback(write_data_replacement);
    config = test_config;
    config_save();
    config = default_config;
    config_load();
    TEST_ASSERT_EQUAL_MEMORY(&test_config, &config, sizeof(config));
}


void test_write_and_read_second_config(void) {
    struct CONFIG new_config;
    write_data_StubWithCallback(write_data_replacement);
    new_config = test_config;
    new_config.axes.accel[0]=5;
    config = test_config;
    config_save();
    config = new_config;
    config_save();
    config = test_config;
    config_load();
    TEST_ASSERT_EQUAL_MEMORY(&new_config, &config, sizeof(config));
}

void test_read_last_config(void) {
    int counter;
    struct CONFIG new_config;
    write_data_StubWithCallback(write_data_replacement);
    counter = MAX_CONFIG_COUNT-1;
    new_config = test_config;
    new_config.axes.accel[0]=5;
    config = test_config;
    while(counter--) {
        config_save();
    }
    config = new_config;
    config_save();
    config = test_config;
    config_load();
    TEST_ASSERT_EQUAL_MEMORY(&new_config, &config, sizeof(config));
}



