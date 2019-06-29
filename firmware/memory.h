//put data at specified place i on pic, otherwise align with 0x800 boundary
#ifdef __XC32
#ifndef __MPLAB_DEBUGGER_SIMULATOR
#define PLACE_DATA_AT(addr) __attribute__((address(addr), space(prog)))
#else
#define PLACE_DATA_AT(addr) __attribute__((aligned(0x800)))
#endif
#else
#define PLACE_DATA_AT(addr) __attribute__((aligned(0x800)))
#endif

#ifndef __DEBUG 
#define CONST_STORE const
#else
#define CONST_STORE
#endif
/**
  @Summary
    Clear any error flags

  @Description
    Resets the error bits in the flash controller
     
  @Param
    No params

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_clear_errors();

/**
  @Summary
    Erase a page of memory

  @Description
    Erase the page referenced by ptr
 
  @Preconditions
    Ptr should point to the start of a memory page
    
  @Param
    ptr: pointer to start of memory to be erased.

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_erase_page(const void *ptr);

/**
  @Summary
    Erase all program flash

  @Description
    Erase all program flash (excluding bootloader)
 
    
  @Param
    No params

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_erase_all();

/**
  @Summary
    Write a row of data to flash

  @Preconditions
    Ptr should point to the start of a memory row
    
  @Param
    ptr: pointer to start of memory to be programmed
    src: the data to be copied over

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_write_row(const void *ptr, const void* src);

/**
  @Summary
    Write a doubleword of data to flash

  @Preconditions
    Ptr should point to the start of doubleword
    
  @Param
    ptr: pointer to start of memory to be programmed
    src: the data to be copied over

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void write_dword(const void *ptr, const void* src);

/**
  @Summary
    Writes data to flash

  @Preconditions
    destination should point to the start of a doubleword
    length should be a multiple of 8
    
  @Param
    ptr: pointer to start of memory to be programmed
    src: the data to be copied over
    length: the number of bytes to be copied

  @Throws
    Throws ERROR_FLASH_STORE_FAILED if there is a problem

*/
void memory_write_data(const void *ptr,  const void *src, int length);

