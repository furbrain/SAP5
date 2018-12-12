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
void erase_page(void *ptr);

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
void erase_memory();

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
void write_row(void *ptr, const void* src);

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
void write_dword(void *ptr, const void* src);

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
void write_data(void *ptr,  const void *src, int length);

