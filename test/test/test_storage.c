#include "unity.h"
#include "exception.h"
#include "storage.h"
#include "mock_memory.h"
#include "mem_locations.h"
#include <stdio.h>
#include <string.h>

const uint32_t target_backup[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
uint32_t target_array[4]; 

int write_dword_replacement(void* ptr, const int* src, int num_calls) {
    int i;
    if ((size_t)ptr % 8) return -1;
    if (((int*)ptr < target_array) || ((int*)ptr > target_array+4)) 
        Throw(0xBADADDA);
    for (i=0; i<8; i++) {
        if (*((uint8_t*)ptr+i) != 0xff) Throw(0xBADDA7A);
    }
    memcpy(ptr, src, 8);
    return 0;
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
