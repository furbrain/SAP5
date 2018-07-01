/* I2C utility funcs*/
#ifndef I2C_UTIL_H
#define I2C_UTIL_H
#include <stdint.h>

#define EEPROM_ADDRESS 0x50
#define I2C_FAST 37
#define I2C_STANDARD 200

void i2c_init(int speed);

int8_t write_i2c_block(uint8_t address, const uint8_t *data, uint8_t length, int speed);

int8_t read_i2c_block(uint8_t address, uint8_t *data, uint8_t length, int speed);

int8_t write_i2c_command_block(uint8_t address, uint8_t command, const uint8_t *data, uint8_t length, int speed);

int8_t write_i2c_data2(uint8_t address, uint8_t command, uint8_t data, int speed);

int8_t read_eeprom_data(uint16_t address, uint8_t *data, uint8_t length);
#ifndef BOOTLOADER
int8_t write_i2c_data1(uint8_t address, uint8_t command, int speed);




int8_t read_i2c_data(uint8_t address, uint8_t command, uint8_t *data, uint8_t length, int speed);

int8_t write_eeprom_data(uint16_t address, const uint8_t *data, uint8_t length);

#endif
#endif
