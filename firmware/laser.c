#include <stdlib.h>
#include <stdio.h>
#include "laser.h"
#include "mcc_generated_files/uart1.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "config.h"

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
        return laser_get_result()+config.calib.laser_offset;
    } else {
        return -2.0;
    }
}

void laser_start(enum LASER_SPEED speed) {
    char text[32];
    int i;
    UART1_ReceiveBufferClear();
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
    if ((22-UART1_ReceiveBufferSizeGet())>=11) {
        if (UART1_Peek(4)=='.') {
            return true;
        }
    }
    return false;
}

double laser_get_result(void) {
    char text[15];
    int read_count, i;
    UART1_ReadBuffer(text, 15);
    text[read_count]=0;
    return atof(text+2);
}

