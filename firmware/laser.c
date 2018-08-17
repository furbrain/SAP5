#include <stdlib.h>
#include <stdio.h>
#include "laser.h"
#include "mcc_generated_files/uart1.h"
#include "utils.h"
#include "display.h"
#include "font.h"

void laser_on(void) {
    UART1_ReceiveBufferClear();
    UART1_Write('O');
}

void laser_off(void) {
    UART1_ReceiveBufferClear();
    UART1_Write('C');
}

double laser_read(enum LASER_SPEED speed, int timeout) {
    laser_start(speed);
    while (!laser_result_ready() && timeout>0) {
        delay_ms(5);
        timeout -= 5;
    }
    if (timeout<=0) return -1.0;
    if (laser_result_ready()) {
        return laser_get_result();
    } else {
        return -2.0;
    }
}

void laser_start(enum LASER_SPEED speed) {
    char text[32];
    int i;
    i = UART1_ReadBuffer(text,32);
    text[i] = 0;
    display_write_text(3,1,text,&small_font,false);
    switch (speed){
        case LASER_SLOW:
            UART1_Write('M');
            break;
        case LASER_MEDIUM:
            UART1_Write('D');
            break;
        case LASER_FAST:
            UART1_Write('F');
            break;
    }
}

bool laser_result_ready(void) {
    if (UART1_ReceiveBufferSizeGet()>=11) {
        if (UART1_Peek(4)=='.') {
            return true;
        }
    }
    return false;
}

double laser_get_result(void) {
    char text[15];
    char d_text[30];
    int read_count, i;
    i = UART1_ReceiveBufferSizeGet();
    read_count = UART1_ReadBuffer(text, 15);
    text[read_count]=0;
    snprintf(d_text, 30, "%d:%d; %s", i, read_count, text);
    display_write_text(1,1,d_text,&small_font,false);
    return atof(text+2);
}

