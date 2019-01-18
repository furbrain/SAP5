#include "beep.h"
#include "mcc_generated_files/mccp2_compare.h"
#include "utils.h"


/* beep at freq for duration milliseconds*/
void beep(double freq, int duration) {
    uint16_t priVal;
    priVal = (uint16_t)(BEEP_FREQ/ freq);
    priVal = (2 * 0x10000 * freq) / BEEP_FREQ;
    
    MCCP2_COMPARE_SingleCompare16ValueSet(priVal);
    MCCP2_COMPARE_Start();
    delay_ms_safe(duration);
    MCCP2_COMPARE_Stop();
}