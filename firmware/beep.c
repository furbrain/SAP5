#include <xc.h>
#include "beep.h"
#include "mcc_generated_files/mccp2_compare.h"
#include "utils.h"

/* initialise beeping circuitry...*/
void beep_initialise(void){
    // ON disabled; MOD 16-Bit Single Edge, Toggle; ALTSYNC disabled; SIDL disabled; OPS Each Time Base Period Match; CCPSLP disabled; TMRSYNC disabled; RTRGEN disabled; CCSEL disabled; ONESHOT disabled; TRIGEN disabled; T32 16 Bit; SYNC None; OPSSRC Timer Interrupt Event; TMRPS 1:4; CLKSEL SYSCLK; 
    CCP2CON1 = 0x43;
    //OCCEN enabled; OCDEN enabled; ASDGM disabled; OCEEN disabled; ICGSM Level-Sensitive mode; OCFEN disabled; ICS ICM2; SSDG disabled; AUXOUT Disabled; ASDG None; OCAEN disabled; OCBEN disabled; OENSYNC disabled; PWMRSEN disabled; 
    CCP2CON2 = 0xC000000;
    //DT 0; OETRIG disabled; OSCNT None; POLACE disabled; POLBDF disabled; PSSBDF Inactive; OUTM Half bridge output; PSSACE Inactive; 
    CCP2CON3 = 0x20A0000;
    //SCEVT disabled; TRSET disabled; ICOV disabled; ASEVT disabled; ICGARM disabled; RBWIP disabled; TRCLR disabled; RAWIP disabled; TMRHWIP disabled; TMRLWIP disabled; PRLWIP disabled; 
    CCP2STAT = 0x0;
    TRISBCLR = 0x03;
}

/* beep at freq for duration milliseconds*/
void beep(double freq, int duration) {
    uint32_t priVal;
    priVal = (BEEP_FREQ/ (freq*2));
    //priVal = (2 * 0x10000 * freq) / BEEP_FREQ;
    CCP2PR = priVal;
    CCP2RA = priVal;
    CCP2CON1bits.ON = 1;
    delay_ms_safe(duration);
    CCP2CON1bits.ON = 0;
}

/* make a happy sounding beep - CEGC*/
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

/*just a beepy beep*/
void beep_beep(void) {
    beep(1000,20);
}
    