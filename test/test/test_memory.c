#include "unity.h"
#include "exception.h"
#include "memory.h"
#include "mock_utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/kmem.h>

#define K2P(addr) ((void*)KVA_TO_PA(addr))

#define FLASH_EXPECT(dest, src, op, ret) utils_flash_memory_ExpectAndReturn(K2P(dest), K2P(src), op, ret);


const uint32_t target_backup[4] __attribute__((aligned(8))) = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
uint32_t __attribute__((aligned(4))) target_array[4]; 

void test_erase_page_normal(void) {
    FLASH_EXPECT(0x9d000800, NULL, FLASH_ERASE_PAGE,0);
    erase_page((void*)0x9d000800);
    TEST_ASSERT_THROWS(erase_page((void*)0x9d000400), ERROR_FLASH_STORE_FAILED);
}



void test_write_data(void) {
    CEXCEPTION_T e;
    char text[80];
    const char *reason;
    const char *file;
    int line;
    int zeros[4] __attribute__((aligned(4)))= {0,0,0,0};
    Try {
        FLASH_EXPECT(&target_backup, &zeros, FLASH_WRITE_DWORD, 0);
        //FLASH_EXPECT(&target_array[2], &zeros[2], FLASH_WRITE_DWORD, 0);
        write_data(&target_backup, &zeros, 8);
    } 
    Catch (e) {
        exception_get_details(&reason, &file, &line);
        TEST_FAIL_MESSAGE(reason);
    }
}

