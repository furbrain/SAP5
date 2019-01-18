#ifndef _BEEP_H
#define _BEEP_H

#define BEEP_FREQ (24000000 / 64)

/* beep at freq for duration milliseconds*/
void beep(double freq, int duration);
#endif // _BEEP_H
