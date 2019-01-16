#include "unity.h"
#include "exception.h"
#include "memory.h"
#include "mock_utils.h"
#include <stdio.h>
#include <string.h>

#include <sys/kmem.h>

#define K2P(addr) ((void*)KVA_TO_PA(addr))

#define FLASH_EXPECT(dest, src, op, ret) utils_flash_memory_ExpectAndReturn(K2P(dest), src, op, ret);


const uint32_t target_backup[4] __attribute__((aligned(8))) = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
uint32_t __attribute__((aligned(4))) target_array[4]; 
int nums[4] __attribute__((aligned(4)))= {1,2,3,4};

void test_erase_page_normal(void) {
    FLASH_EXPECT(0x9d000800, NULL, FLASH_ERASE_PAGE,0);
    erase_page((void*)0x9d000800);
}

void test_erase_page_throws_error_with_non_boundary_input(void) {
    TEST_ASSERT_THROWS(erase_page((void*)0x9d000400), ERROR_FLASH_STORE_FAILED);
}

void test_erase_page_throws_error_if_fails(void) {
    FLASH_EXPECT(0x9d000800, NULL, FLASH_ERASE_PAGE,1);
    TEST_ASSERT_THROWS(erase_page((void*)0x9d000800), ERROR_FLASH_STORE_FAILED);
}

void test_erase_chip_normal(void) {
    FLASH_EXPECT(NULL, NULL, FLASH_ERASE_CHIP, 0);
    erase_memory();
}

void test_erase_chip_throws_error_if_fails(void) {
    FLASH_EXPECT(NULL, NULL, FLASH_ERASE_CHIP,1);
    TEST_ASSERT_THROWS(erase_memory(), ERROR_FLASH_STORE_FAILED);
}

void test_write_dword_normal(void) {
    FLASH_EXPECT(0x9d000800, &target_backup, FLASH_WRITE_DWORD,0);
    write_dword((void*)0x9d000800, &target_backup);
}

void test_write_dword_throws_error_with_non_boundary_input(void) {
    TEST_ASSERT_THROWS(write_dword((void*)0x9d000404, &target_backup), ERROR_FLASH_STORE_FAILED);
}

void test_write_dword_throws_error_if_fails(void) {
    FLASH_EXPECT(0x9d000800, &target_backup, FLASH_WRITE_DWORD,1);
    TEST_ASSERT_THROWS(write_dword((void*)0x9d000800, &target_backup), ERROR_FLASH_STORE_FAILED);
}

void test_write_row_normal(void) {
    FLASH_EXPECT(0x9d000800, K2P(&target_backup), FLASH_WRITE_ROW,0);
    write_row((void*)0x9d000800, &target_backup);
}

void test_write_row_throws_error_with_non_boundary_input(void) {
    TEST_ASSERT_THROWS(write_row((void*)0x9d000480, K2P(&target_backup)), ERROR_FLASH_STORE_FAILED);
}

void test_write_row_throws_error_if_fails(void) {
    FLASH_EXPECT(0x9d000800, K2P(&target_backup), FLASH_WRITE_ROW,1);
    TEST_ASSERT_THROWS(write_row((void*)0x9d000800, &target_backup), ERROR_FLASH_STORE_FAILED);
}

void test_write_data_16_bytes(void) {
    FLASH_EXPECT(&target_backup, &nums, FLASH_WRITE_DWORD, 0);
    FLASH_EXPECT(&target_backup[2], &nums[2], FLASH_WRITE_DWORD, 0);
    write_data((void*)&target_backup, &nums, 16);
}

void test_write_data_2_rows(void) {
    FLASH_EXPECT((void*)0x9d000800, K2P(&nums), FLASH_WRITE_ROW, 0);
    FLASH_EXPECT((void*)0x9d000900, K2P((void*)nums + 0x100), FLASH_WRITE_ROW, 0);
    write_data((void*)0x9d000800, &nums, 0x200);
}

void test_write_data_1_row_2_dwords(void) {
    FLASH_EXPECT((void*)0x9d0007F8, &nums, FLASH_WRITE_DWORD, 0);
    FLASH_EXPECT((void*)0x9d000800, K2P((void*)nums + 0x08), FLASH_WRITE_ROW, 0);
    FLASH_EXPECT((void*)0x9d000900, (void*)nums + 0x108, FLASH_WRITE_DWORD, 0);
    write_data((void*)0x9d0007F8, &nums, 0x110);
}

void test_write_data_fails_with_non_boundary_input(void) {
    TEST_ASSERT_THROWS(write_data((void*)0x9000404, &nums, sizeof(nums)), ERROR_FLASH_STORE_FAILED);
}

void test_write_data_fails_with_wrong_length(void) {
    TEST_ASSERT_THROWS(write_data((void*)0x9000800, &nums, 12), ERROR_FLASH_STORE_FAILED);
}

void test_write_data_fails_with_non_word_source(void) {
    TEST_ASSERT_THROWS(write_data((void*)0x9000800, (void*)&nums+2, 16), ERROR_FLASH_STORE_FAILED);
}


