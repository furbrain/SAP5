#include "mcc_generated_files/tmr1.h"

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