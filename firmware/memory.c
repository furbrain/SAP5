#include <sys/kmem.h>
#include <stddef.h>
#include <stdint.h>
#include "app_type.h"
#ifndef BOOTLOADER
#include "exception.h"
#else
#define THROW_WITH_REASON(reason, code) {}
#endif
#include "memory.h"
#include "utils.h"

#define PAGE_SIZE 0x0800
#define ROW_SIZE 0x0100
#define DWORD_SIZE 0x0008

/**
  @Summary
    Clear any error flags

  @Description
    Resets the error bits in the flash controller
     
  @Param
    No params

  @ThrowsO
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_clear_errors() {
    unsigned int res;
    res = utils_flash_memory(NULL, NULL, FLASH_CLEAR_ERROR);
    if (res) {
        THROW_WITH_REASON("Clear errors failed", ERROR_FLASH_STORE_FAILED);
    }    
}


/**
  @Summary
    Erase a page of memory

  @Description
    Erase the page referenced by ptr
 
  @Preconditions
    Ptr should point to the start of a memory page
    
  @Param
    ptr: pointer to start of memory to be erased.

  @Returns
    returns 0 on success, -1 if ptr is not on a page boundary, -2 if an error in
    erasing

*/
void memory_erase_page(const void *ptr) {
    unsigned int res;
    size_t page;
    page = (size_t)KVA_TO_PA(ptr);
    if (page % PAGE_SIZE) {
        THROW_WITH_REASON("Erase page not on page boundary", ERROR_FLASH_STORE_FAILED);
    }
    res = utils_flash_memory((void*)page, NULL, FLASH_ERASE_PAGE);
    if (res) {
        THROW_WITH_REASON("Erase page failed", ERROR_FLASH_STORE_FAILED);
    }
}

/**
  @Summary
    Erase all program flash

  @Description
    Erase all program flash (excluding bootloader)
 
    
  @Param
    No params

  @Returns
    returns 0 on success, -2 if an error on erase

*/
void memory_erase_all() {
    unsigned int res;
    res = utils_flash_memory(NULL, NULL, FLASH_ERASE_CHIP);
    if (res) {
        THROW_WITH_REASON("Erase chip failed", ERROR_FLASH_STORE_FAILED);
    }
}

/**
  @Summary
    Write a row of data to flash

  @Preconditions
    Ptr should point to the start of a memory row
    
  @Param
    ptr: pointer to start of memory to be programmed
    src: the data to be copied over

  @Returns
    returns 0 on success, -1 if ptr is not on a row boundary, -2 if write 
    fails

*/
void memory_write_row(const void *ptr, const void* src) {
    unsigned int res;
    size_t row;
    row = (size_t)KVA_TO_PA(ptr);
    src = (const void*) KVA_TO_PA(src);
    if (row % ROW_SIZE) {
        THROW_WITH_REASON("Write row not on row boundary", ERROR_FLASH_STORE_FAILED);
    }
    res = utils_flash_memory((void*)row, src, FLASH_WRITE_ROW);
    if (res) {
        THROW_WITH_REASON("Write row failed", ERROR_FLASH_STORE_FAILED);
    }
}

/**
  @Summary
    Write a doubleword of data to flash

  @Preconditions
    Ptr should point to the start of doubleword
    
  @Param
    ptr: pointer to start of memory to be programmed
    src: the data to be copied over

  @Returns
    returns 0 on success, -1 if ptr is not on a dword boundary, -2 if write 
    fails

*/
void memory_write_dword(const void *ptr, const void* src){
    unsigned int res;
    size_t dword;
    // Load data into NVMDATA register
    dword = (size_t)KVA_TO_PA(ptr);
    if (dword % DWORD_SIZE) {
        THROW_WITH_REASON("Write address not on doubleword boundary", ERROR_FLASH_STORE_FAILED);
    }
    res = utils_flash_memory((void*)dword, src, FLASH_WRITE_DWORD);
    if (res) {
        THROW_WITH_REASON("Write dword failed", ERROR_FLASH_STORE_FAILED);
    }
}

void memory_write_data(const void *ptr,  const void *src, int length){
    if ((size_t)ptr % 8)
        THROW_WITH_REASON("Write address not on doubleword boundary", ERROR_FLASH_STORE_FAILED);
    if (length % 8)
        THROW_WITH_REASON("Length not a multiple of 8 bytes", ERROR_FLASH_STORE_FAILED);
    if ((size_t)src % 4)
        THROW_WITH_REASON("Source address not on word boundary", ERROR_FLASH_STORE_FAILED);
#pragma GCC diagnostic ignored "-Wpointer-arith"    
    while (length > 0) {
        if ((((size_t)ptr % ROW_SIZE) ==0) && (length >= ROW_SIZE)) {
            memory_write_row(ptr, src);
            ptr += ROW_SIZE;
            src += ROW_SIZE;
            length -= ROW_SIZE;            
        } else {
            memory_write_dword(ptr, src);
            ptr += DWORD_SIZE;
            src += DWORD_SIZE;
            length -= DWORD_SIZE;
        }
    }
#pragma GCC diagnostic pop
}
