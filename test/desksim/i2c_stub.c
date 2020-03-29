#include <zmq.h>
#include <string.h>
#include "unity.h"
#include "zmqstubs.h"
#include "mock_i2c1.h"


void i2c_write_stub(uint8_t *data, uint8_t length, uint16_t address, I2C1_MESSAGE_STATUS *status, int numcalls) {
    char buffer[300];
    buffer[0] = 0x01;
    buffer[1] = (char)address*2;
    memcpy(buffer+2, data, length);
    zmq_send(requester, buffer, length+2, 0);
    zmq_recv(requester, buffer, 300, 0);
    if (buffer[0]==0x01 && buffer[1]==0x00) {
        *status = I2C1_MESSAGE_COMPLETE;
    } else {
        *status = I2C1_MESSAGE_FAIL;
    }
}


void i2c_read_stub(uint8_t *data, uint8_t length, uint16_t address, I2C1_MESSAGE_STATUS *status, int numcalls) {
    char buffer[300];
    int count;
    buffer[0] = 0x02;
    buffer[1] = (char)address * 2 + 1;
    buffer[2] = length;
    zmq_send(requester, buffer, 3, 0);
    count = zmq_recv(requester, buffer, 300, 0);
    memcpy(data, &buffer[1], count-1);
    if (buffer[0]==0x02) {
        *status = I2C1_MESSAGE_COMPLETE;
    } else {
        *status = I2C1_MESSAGE_FAIL;
    }
}


void i2c_setup() {
    I2C1_Initialize_Ignore();
    I2C1_MasterWrite_StubWithCallback(i2c_write_stub);  
    I2C1_MasterRead_StubWithCallback(i2c_read_stub);  
}

