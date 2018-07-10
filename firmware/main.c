#include <stdio.h>

#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "sensors.h"

#define TXT_LENGTH 50

void main(void)
{
    
    struct RAW_SENSORS sensors;
    char text[TXT_LENGTH];
    int i;
    SYSTEM_Initialize();
    PERIPH_EN_SetHigh();
    delay_ms(1);
    display_init();
    display_clear_screen();
    while (1)
    {
        sensors_read_raw(&sensors,false);
        i = getsUSBUSART(text, TXT_LENGTH);
        if (i) {
            //display_clear_screen();
            display_write_text(1,1,text, &small_font, false);
        }
        delay_ms(100);
        
    }
}