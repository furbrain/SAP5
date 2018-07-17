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
    char text[TXT_LENGTH-1];
    int i;
    SYSTEM_Initialize();
    PERIPH_EN_SetHigh();
    delay_ms(1000);
    display_init();
    display_clear_screen();
    display_write_text(4,1,"On",&large_font,false);
    while (1)
    {
        show_menu(get_menu_item_offset(0),true);
    }
}