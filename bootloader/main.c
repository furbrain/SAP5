/*
 * M-Stack USB Bootloader
 *
 *  M-Stack is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, version 3; or the Apache License, version 2.0
 *  as published by the Apache Software Foundation.  If you have purchased a
 *  commercial license for this software from Signal 11 Software, your
 *  commerical license superceeds the information in this header.
 *
 *  M-Stack is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this software.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  You should have received a copy of the Apache License, verion 2.0 along
 *  with this software.  If not, see <http://www.apache.org/licenses/>.
 *
 * Alan Ott
 * Signal 11 Software
 * 2013-05-03
 */
/* define processor clock speed for use in delay funcs*/
#include <xc.h>
#include <string.h>
#include "config.h"
#include "mcc_generated_files/mcc.h"
#include "usb/usb.h"
#include "memory.h"
#include "battery.h"
#include "i2c_util.h"
#include "utils.h"

/* Variables from linker script.
 * 
 * The way to pass values from the linker script to the program is to create
 * variables in the linker script and extern them in the program.  The only
 * catch is that assignment operations in the linker script only set
 * variable _addresses_, and not variable values.  Thus to get the data in
 * the program, you need to take the _address_ of the variables created by
 * the linker script (and ignore their actual values).  This may seem hacky,
 * but the GNU LD manual cites it as the recommended way to do it.  See
 * section 3.5.5 "Source Code Reference."
 *
 * It's also worth noting that on Extended Data Space (EDS) PIC24F parts,
 * the linker will but a 0x01 in the high byte of each of these addresses. 
 * Unfortunately, it's impossible with GCC to take the address of a
 * varialbe, apply a mask (in our cause we'd want to use 0x00ffffff), and
 * use that value as an initializer for a constant.  Becaause of this, the
 * assignment of the uint32_t "constants" below has to be done in main().
 */

/* Flash block(s) where the bootloader resides.*/
#define USER_REGION_BASE  IVT_MAP_BASE
#define USER_REGION_TOP   (FLASH_TOP - FLASH_BLOCK_SIZE) /* The last page has the config words. Don't clear that one*/


/* Instruction sizes */
#ifdef __PIC24F__
	#define INSTRUCTIONS_PER_ROW 64
	#define BYTES_PER_INSTRUCTION 4
	#define WORDS_PER_INSTRUCTION 2
#else
    #define INSTRUCTIONS_PER_ROW 64
    #define BYTES_PER_INSTRUCTION 4
    #define WORDS_PER_INSTRUCTION 2
#endif

#define BUFFER_LENGTH (INSTRUCTIONS_PER_ROW * WORDS_PER_INSTRUCTION)

/* Protocol commands */
#define CLEAR_FLASH 100
#define SEND_DATA 101
#define GET_CHIP_INFO 102
#define REQUEST_DATA 103
#define SEND_RESET 105

/* I2C commands */
#define WRITE_I2C_DATA 110
#define READ_I2C_DATA 111
#define CHECK_I2C_READY 112
#define WRITE_DISPLAY 113
#define READ_EEPROM_DATA 114
#define DISPLAY_ADDRESS 0x3C
/*datetime commands*/
#define WRITE_DATETIME 120
#define READ_DATETIME 121

struct chip_info {
	uint32_t user_region_base;
	uint32_t user_region_top;
	uint32_t config_words_base;
	uint32_t config_words_top;

	uint8_t bytes_per_instruction;
	uint8_t instructions_per_row;
	uint8_t pad0;
	uint8_t pad1;
};

/* Data-to-program: buffer and attributes. */
static uint32_t write_address; /* program space word address */
static size_t write_length;    /* number of words, not bytes */
static uint16_t prog_buf[BUFFER_LENGTH];
static uint8_t i2c_on;

static struct chip_info chip_info = { };

void clear_flash()
{
    erase_memory();
}

void write_flash_row()
{
    uint16_t data[BUFFER_LENGTH];
    memcpy(data,prog_buf, write_length);
    memset(data+write_length*2,0xff,(BUFFER_LENGTH-write_length)*2);
    write_row((void*)write_address,data);
}


/* Read data starting at prog_addr into the global prog_buf*/
static void read_prog_data(uint32_t prog_addr, uint32_t len/*bytes*/)
{
    memcpy((void *)prog_buf, (void*)prog_addr, len);
}


static int8_t write_i2c(uint8_t device_address, uint8_t* buffer, uint16_t len) {
	return write_i2c_block(device_address,buffer,len);
}

static int8_t write_display(uint8_t page, uint8_t column, uint8_t* buffer, uint16_t len) {
    write_i2c_data1(DISPLAY_ADDRESS,0xB0+page);
    write_i2c_data1(DISPLAY_ADDRESS,column & 0x0F);
    write_i2c_data1(DISPLAY_ADDRESS,16 + (column / 16));
    return write_i2c_command_block(DISPLAY_ADDRESS,0x40,buffer,len);
}


uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void display_show_bat(int charge) {
	static int counter = 0;
	int i;
	// set page and column to top right
	char bat_status[24];
	if (charge==-1) {
		charge = counter;
		counter++;
		if (counter>18) counter = 0;
	}
	bat_status[0] = 0xf8;
	bat_status[1] = 0x04;
	memset(&bat_status[2],0xf4,charge);
	memset(&bat_status[2+charge],0x04,19-charge);
	bat_status[21] = 0xf8;
	bat_status[22] = 0x20;
	bat_status[23] = 0xC0;
	render_data_to_page(0,104,bat_status,24);
	for (i = 0; i< 24; ++i) {
		bat_status[i] = reverse(bat_status[i]);
	}
	render_data_to_page(1,104,bat_status,24);
}

int main(void)
{
	uint32_t pll_startup_counter = 600;
	int display_initialised = 0;
	int counter = 0;
	enum BAT_STATUS bat_status;
    SYSTEM_Initialize();
	/* setup ports */
	/* enable peripherals */
	/* first look to see if we should be running bootloader at all... */
	display_init();
	display_clear_screen();
	usb_init();
	delay_ms(3);
	while (1) {
		bat_status = get_bat_status();
		if (bat_status==DISCHARGING) break;
		counter++;
		if ((counter & 0x3fff)==0) {
			// only update display every 32 cycles
			if (bat_status==CHARGING) display_show_bat(-1);
			if (bat_status==CHARGED) display_show_bat(18);
		}
        usb_service();
	}
	/* Jump to application */
	__asm__("j %0"
		: /* no outputs */
		: "r" (IVT_MAP_BASE)
		: /* no clobber*/);

	return 0;
}

static void empty_cb(bool transfer_ok, void *context)
{
	/* Nothing to do here. */
}

static void reset_cb(bool transfer_ok, void *context)
{
	/* Delay before resetting*/
	uint16_t i = 65535;
	while(i--)
		;
    abort();
}

static void write_data_cb(bool transfer_ok, void *context)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in buf[]. */

	if (transfer_ok)
		write_flash_row();
}

static void write_i2c_cb(bool transfer_ok, void *context)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in buf[]. */

	if (transfer_ok)
		write_i2c(write_address,(char*)prog_buf,write_length);
}

static void write_display_cb(bool transfer_ok, void *context)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in buf[]. */

	if (transfer_ok) {
		render_data_to_page(write_address>>16,write_address&0xFF,(char*)prog_buf,write_length);
	}
}

static void write_datetime_cb(bool transfer_ok, void *context) {
	if (transfer_ok) {
		RTCC_TimeSet((struct tm*)prog_buf);
	}
}


int8_t app_unknown_setup_request_callback(const struct setup_packet *setup)
{
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

	/* This handler handles request 254/dest=other/type=vendor only.*/
	if (setup->REQUEST.destination == DEST_OTHER_ELEMENT &&
	    setup->REQUEST.type == REQUEST_TYPE_VENDOR &&
	    setup->REQUEST.direction == 0/*OUT*/) {

		if (setup->bRequest == CLEAR_FLASH) {
			/* Clear flash Request */
			clear_flash();
			
			/* There will be NO data stage. This sends back the
			 * STATUS stage packet. */
			usb_send_data_stage(NULL, 0, empty_cb, NULL);
		}
		else if (setup->bRequest == SEND_DATA) {
			/* Write Data Request */
			if (setup->wLength > sizeof(prog_buf))
				return -1;

			write_address = setup->wValue | ((uint32_t) setup->wIndex) << 16;
			write_address /= 2; /* Convert to word address. */
			write_length = setup->wLength;
			write_length /= 2;  /* Convert to word length. */

			/* Make sure it is within writable range (ie: don't
			 * overwrite the bootloader or config words). */
			if (write_address < USER_REGION_BASE)
				return -1;
			if (write_address + write_length > USER_REGION_TOP)
				return -1;

			/* Check for overflow (unlikely on known MCUs) */
			if (write_address + write_length < write_address)
				return -1;

			/* Check length */
			if (setup->wLength > sizeof(prog_buf))
				return -1;

			memset(prog_buf, 0xff, sizeof(prog_buf));
			usb_start_receive_ep0_data_stage((char*)prog_buf, setup->wLength, &write_data_cb, NULL);
		}
		else if (setup->bRequest == SEND_RESET) {
			/* Reset to Application Request*/

			/* There will be NO data stage. This sends back the
			 * STATUS stage packet. */
			usb_send_data_stage(NULL, 0, reset_cb, NULL);
		}
		else if (setup->bRequest == WRITE_I2C_DATA) {
			write_address = setup->wValue;

			/* Check length */
			if (setup->wLength > sizeof(prog_buf))
				return -1;

			write_length = setup->wLength;
			usb_start_receive_ep0_data_stage((char*)prog_buf, setup->wLength, &write_i2c_cb, NULL);
		}
		else if (setup->bRequest == WRITE_DISPLAY) {
			write_address = setup->wValue | ((uint32_t) setup->wIndex) << 16;

			/* Check length */
			if (setup->wLength > sizeof(prog_buf))
				return -1;
			write_length = setup->wLength;
			usb_start_receive_ep0_data_stage((char*)prog_buf, setup->wLength, &write_display_cb, NULL);
            
		}
		else if (setup->bRequest == WRITE_DATETIME) {
			if (setup->wLength != sizeof(struct tm))
				return -1;
			usb_start_receive_ep0_data_stage((char*)prog_buf, setup->wLength, &write_datetime_cb, NULL);			
		}
	}

	if (setup->REQUEST.destination == DEST_OTHER_ELEMENT &&
	    setup->REQUEST.type == REQUEST_TYPE_VENDOR &&
	    setup->REQUEST.direction == 1/*IN*/) {

		if (setup->bRequest == GET_CHIP_INFO) {
			/* Request Device Info Struct */
			chip_info.user_region_base = USER_REGION_BASE * 2;
			chip_info.user_region_top = USER_REGION_TOP * 2;
			chip_info.config_words_base = CONFIG_WORDS_BASE * 2;
			chip_info.config_words_top = CONFIG_WORDS_TOP * 2;

			chip_info.bytes_per_instruction = BYTES_PER_INSTRUCTION;
			chip_info.instructions_per_row = INSTRUCTIONS_PER_ROW;

			usb_send_data_stage((char*)&chip_info, sizeof(struct chip_info), empty_cb/*TODO*/, NULL);
		}

		if (setup->bRequest == REQUEST_DATA) {
			/* Request program data */
			uint32_t read_address;

			read_address = setup->wValue | ((uint32_t) setup->wIndex) << 16;
			read_address /= 2;

			/* Range-check address */
			if (read_address + setup->wLength > FLASH_TOP)
				return -1;

			/* Check for overflow (unlikely on known MCUs) */
			if (read_address + setup->wLength < read_address)
				return -1;

			/* Check length */
			if (setup->wLength > sizeof(prog_buf))
				return -1;

			
			read_prog_data(read_address, setup->wLength / 2);
			usb_send_data_stage((char*)prog_buf, setup->wLength, empty_cb/*TODO*/, NULL);
		}
		else if (setup->bRequest == READ_I2C_DATA) {
			write_address = setup->wValue;
			if (setup->wLength > sizeof(prog_buf))
 				return -1;
			read_i2c_block(setup->wValue,(uint8_t*)prog_buf,setup->wLength);
			usb_send_data_stage((char*)prog_buf, setup->wLength, empty_cb/*TODO*/, NULL);
		}
		else if (setup->bRequest == READ_DATETIME) {
			if (setup->wLength != sizeof(struct tm))
				return -1;
			RTCC_TimeGet((struct tm*)prog_buf);
			usb_send_data_stage((char*)prog_buf, setup->wLength, empty_cb/*TODO*/,NULL);
		}
	}

	return 0; /* 0 = can handle this request. */
#undef MIN
}

void app_usb_reset_callback(void)
{

}
