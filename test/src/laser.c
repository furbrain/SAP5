#include <stdlib.h>
#include <stdio.h>
#include "laser.h"
#include "mcc_generated_files/uart1.h"
#include "utils.h"
#include "display.h"
#include "font.h"
#include "config.h"
#include "exception.h"

TESTABLE_STATIC
bool laser_result_ready(void) {
    if ((32-UART1_ReceiveBufferSizeGet())>=6) {
        delay_ms_safe(100); //wait to load rest of buffer
        return true;
    }
    return false;
}

TESTABLE_STATIC
double laser_get_result(void) {
    char text[15];
    char *error;
    int read_count;
    double result;
    read_count = UART1_ReadBuffer((uint8_t*)text, 15);
    text[read_count]=0;
    result = strtod(text+2, &error);
    if (error==text+2) {
        THROW_WITH_REASON("Laser returned gibberish", ERROR_LASER_READ_FAILED);
    }
    return result;
}

TESTABLE_STATIC
void laser_start(enum LASER_SPEED speed) {
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

void laser_on() {
    UART1_ReceiveBufferClear();
    UART1_Write('O');
}

void laser_off() {
    UART1_ReceiveBufferClear();
    UART1_Write('C');
}


double laser_read_raw(enum LASER_SPEED speed, int timeout) {
    laser_start(speed);
    while (!laser_result_ready() && timeout>0) {
        delay_ms_safe(5);
        timeout -= 5;
    }
    if (timeout<=0){
        THROW_WITH_REASON("Timed out", ERROR_LASER_TIMEOUT);
    }
    return laser_get_result();
}

double laser_read(enum LASER_SPEED speed, int timeout) {
    return laser_read_raw(speed, timeout)+config.calib.laser_offset;
}
