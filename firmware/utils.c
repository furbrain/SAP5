#include "mcc_generated_files/mcc.h"
#include "utils.h"

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

