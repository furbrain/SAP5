#include "mcc_generated_files/i2c1.h"
#include "i2c_util.h"
#include "utils.h"


#include <string.h>

#define I2C_TIMEOUT 10
#define I2C_RETRIES 10

void i2c_init(){
    I2C1_Initialize();
}

void i2c_close(){
    /* FIXME */
    //CloseI2C();
}

int8_t write_i2c_block(uint8_t address, uint8_t *data, uint8_t length) {
    I2C1_MESSAGE_STATUS status = I2C1_MESSAGE_PENDING;
    int timeout = I2C_TIMEOUT;
    int retries = I2C_RETRIES;
    while(status != I2C1_MESSAGE_FAIL) {
        // write one byte to EEPROM (3 is the number of bytes to write)
        I2C1_MasterWrite(data, length, address, &status);
        // wait for the message to be sent or status has changed.
        while(status == I2C1_MESSAGE_PENDING)
        {
            delay_ms(1);
            // timeout checking
            // check for max retry and skip this byte
            if (timeout <= 0)
                break;
            else
                timeout--;
        } 
        if ((timeout <= 0) || (status == I2C1_MESSAGE_COMPLETE))
            break;

        // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
        //               or I2C1_DATA_NO_ACK,
        // The device may be busy and needs more time for the last
        // write so we can retry writing the data, this is why we
        // use a while loop here

        // check for max retry and skip this byte
        if (retries <= 0)
            break;
        else
            retries--;
    }
    if (status ==I2C1_MESSAGE_COMPLETE) {
        return 0;
    } else {
        return -1;
    }
}

int8_t write_i2c_command_block(uint8_t address, uint8_t command, const uint8_t *data, uint8_t length) {
    uint8_t new_data[256];
    new_data[0] = command;
    memcpy(new_data+1,data,length);
    return write_i2c_block(address, new_data, length+1);
}

int8_t read_i2c_block(uint8_t address, uint8_t *data, uint8_t length) {
    I2C1_MESSAGE_STATUS status = I2C1_MESSAGE_PENDING;
    int timeout = I2C_TIMEOUT;
    int retries = I2C_RETRIES;
    while(status != I2C1_MESSAGE_FAIL) {
        I2C1_MasterRead(data, length, address, &status);
        while(status == I2C1_MESSAGE_PENDING)
        {
            delay_ms(1);
            if (timeout <= 0)
                break;
            else
                timeout--;
        } 
        if ((timeout <= 0) || (status == I2C1_MESSAGE_COMPLETE))
            break;

        // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
        //               or I2C1_DATA_NO_ACK,
        // The device may be busy and needs more time for the last
        // write so we can retry writing the data, this is why we
        // use a while loop here

        // check for max retry and skip this byte
        if (retries <= 0)
            break;
        else
            retries--;
    }
    if (status == I2C1_MESSAGE_COMPLETE) {
        return 0;
    } else {
        return -1;
    }
}


int8_t write_i2c_data2(uint8_t address, uint8_t command, uint8_t data) {
	uint8_t data_list[2];
	data_list[0] = command;
	data_list[1] = data;
	return write_i2c_block(address,data_list,2);
}

int8_t write_i2c_data1(uint8_t address, uint8_t command) {
	return write_i2c_block(address, &command, 1);
}
#ifndef BOOTLOADER


int8_t read_i2c_data(uint8_t address, uint8_t command, uint8_t *data, uint8_t length) {
    if (write_i2c_data1(address, command)==0)
        if(read_i2c_block(address, data, length)==0)
            return 0;
    return -1;
}
#endif
