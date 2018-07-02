#include "mcc_generated_files/mcc.h"
#include "utils.h"
#include "display.h"
#include "font.h"

void main(void)
{
    SYSTEM_Initialize();
    INTERRUPT_GlobalDisable();
    PERIPH_EN_SetHigh();
    delay_ms(1000);
    display_init();
    delay_ms(1000);
    while (1)
    {
        display_write_text(1,1,"Hello", &small_font, false);
    }
}