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
    delay_ms(1000);
    display_init();
    display_clear_screen();
    delay_ms(1000);
    while (1)
    {
        sensors_read_raw(&sensors,false);
        display_clear_screen();
        for (i=0; i< 3; i++){
            snprintf(text, TXT_LENGTH, "%d", sensors.accel[i]);
            display_write_text(i*2,1,text, &small_font, false);
        }
        delay_ms(500);
    }
}