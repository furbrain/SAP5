#include <xc.h>
#include <time.h>
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/rtcc.h"
#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "exception.h"

void delay_ms(int count) {
    while (count > 0) {
        TMR1_Counter16BitSet(0);
        TMR1_Start();
        while (!TMR1_GetElapsedThenClear()) {
            TMR1_Tasks_16BitOperation();
        }
        count--;
    }
}

void delay_ms_safe(int count) {
    while (count > 500) {
        wdt_clear();
        delay_ms(500);
        count -=500;
    }
    wdt_clear();
    delay_ms(count);
    wdt_clear();
}

/* get current time in time_t format */
time_t utils_get_time() {
    struct tm temp_time;
    RTCC_TimeGet(&temp_time);
    return mktime(&temp_time);
}


void wdt_clear(void){
    volatile unsigned short *wdtKey;
    wdtKey = (unsigned short*) (&WDTCON)+1;
    *wdtKey = 0x5743;
}


void sys_reset(int32_t a){
    /* The following reset procedure is from the Family Reference Manual,
	 * Chapter 7, "Resets," under "Software Resets." */
    int x;
	/* Unlock sequence */
	SYSKEY = 0x00000000;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;

	/* Set the reset bit and then read it to trigger the reset. */
	RSWRSTSET = 1;
	x = RSWRST;

	/* Required NOP instructions */
	asm("nop\n nop\n nop\n nop\n");
}

int utils_flash_memory (void *dest, const void *data, enum FLASH_OP op) {
    int status;
    // Fill out relevant registers
    NVMADDR = (uint32_t)dest;
    switch (op) {
        case FLASH_WRITE_DWORD:
            NVMDATA0 = *((uint32_t*)data);
            NVMDATA1 = *(((uint32_t*)data)+1);
            break;
        case FLASH_WRITE_ROW:
        case FLASH_ERASE_PAGE:
            NVMSRCADDR = (uint32_t)data;
            break;
        case FLASH_ERASE_CHIP:
            break;
        default:
            THROW_WITH_REASON("Invalid flash operation",ERROR_FLASH_STORE_FAILED);
    }
    // Suspend or Disable all Interrupts
    INTERRUPT_GlobalDisable();
    // Enable Flash Write/Erase Operations and Select
    // Flash operation to perform
    NVMCON = op;
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


