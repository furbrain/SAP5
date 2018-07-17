#include <stdio.h>

#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "interface.h"

#define TXT_LENGTH 50

void main(void)
{
    
    struct RAW_SENSORS sensors;
    char text[TXT_LENGTH];
    int i;
    enum ACTION action;
    SYSTEM_Initialize();
    PERIPH_EN_SetHigh();
    TMR2_Start();
    delay_ms(1000);
    display_init();
    display_clear_screen();
    display_write_text(4,1,"On",&large_font,false);
    while (1)
    {
    	for (i=0; i<16; i++) {
	    	display_setbuffer_xy(i,i);
    		display_show_buffer();
    	}
    	display_clear_screen();
    }
}
