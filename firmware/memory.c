#include <xc.h>
#include <stdlib.h>
#include "mcc_generated_files/mcc.h"
#include "memory.h"


unsigned int NVMUnlock (unsigned int nvmop) {
    unsigned int status;
    // Suspend or Disable all Interrupts
    INTERRUPT_GlobalDisable();
    // Enable Flash Write/Erase Operations and Select
    // Flash operation to perform
    NVMCON = nvmop;
    // Write Keys
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    // Start the operation using the Set Register
    NVMCONSET = 0x8000;
    // Wait for operation to complete
    while (NVMCON & 0x8000);
    // Restore Interrupts
    INTERRUPT_GlobalEnable();
    // Disable NVM write enable
    NVMCONCLR = 0x0004000;
    // Return WRERR and LVDERR Error Status Bits
    return (NVMCON & 0x3000);
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
int erase_page(void *ptr) {
    unsigned int res;
    unsigned int page;
    page = (unsigned int) ptr & 0x1fffffff;
    if (page % 2048) return -1;
    // Set NVMADDR to the Start Address of page to erase
    NVMADDR = page;
    // Unlock and Erase Page
    res = NVMUnlock(0x4004);
    // Return Result
    if (res) return -2;
    return 0;
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
int erase_memory() {
    unsigned int res;
    // Unlock and Erase Program Flash
    res = NVMUnlock(0x4005);
    // Return Result
    if (res) return -2;
    return 0;
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
int write_row(void *ptr, const void* src) {
    unsigned int res;
    unsigned int row;
    row = (unsigned int)ptr & 0x1fffffff;
    if (row % 256) return -1;
    // Set NVMADDR to Start Address of row to program
    NVMADDR = row;
    // Set NVMSRCADDR to the SRAM data buffer Address
    NVMSRCADDR = (unsigned int) src;
    // Unlock and Write Row
    res = NVMUnlock(0x4003);
    // Return Result
    if (res) return -2;
    return 0;
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
int write_dword(void *ptr, const int* src){
    unsigned int res;
    unsigned int dword;
    // Load data into NVMDATA register
    dword = (unsigned int)ptr & 0x1fffffff;
    if (dword % 8) return -1;
    NVMDATA0 = src[0];
    NVMDATA1 = src[1];
    // Load address to program into NVMADDR register
    NVMADDR = dword;
    // Unlock and Write Word
    res = NVMUnlock (0x4010);
    // Return Result
    if (res) return -2;
    return 0;
}