#ifndef _BEEP_H
#define _BEEP_H
#include "app_type.h"

#define BEEP_FREQ (24000000 / 4)


/* make a beepy beep */
void beep_beep(void);

#ifndef BOOTLOADER
/* make a happy sounding beep - CEGC*/
void beep_happy(void);

/*make a sad sounding beep - FBB*/
void beep_sad(void);


/* a finishing beep  - CGEC */
void beep_finish(void);
#endif
#endif // _BEEP_H
