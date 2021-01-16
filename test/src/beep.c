#include <xc.h>
#include "beep.h"
#include "utils.h"

/*just a beepy beep*/
void beep_beep(void) {
    beep(1000,20);
}

#ifndef BOOTLOADER

void beep_happy(void) {
    beep(523.251, 30); //C
    beep(659.255, 30); //E
    beep(783.991, 30); //G
    beep(1046.50, 30); //C    
}

/*make a sad sounding beep - FBB*/
void beep_sad(void) {
    beep(698.456, 200);
    beep(493.883, 800);
}


void beep_finish(void) {
    beep(1046.50, 30); //C    
    beep(783.991, 30); //G
    beep(659.255, 30); //E
    beep(523.251, 30); //C
}
#endif