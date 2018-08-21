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
    returns 0 on success, -1 if ptr is not on a page boundary

*/
int erase_page(void *ptr);

/**
  @Summary
    Erase all program flash

  @Description
    Erase all program flash (excluding bootloader)
 
    
  @Param
    No params

  @Returns
    returns 0 on success, -1 if ptr is not on a page boundary

*/
int erase_memory();

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
int write_row(void *ptr, const void* src);

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
int write_dword(void *ptr, const int* src);


const struct CONFIG* read_config(void);

int write_config(struct CONFIG *config); 