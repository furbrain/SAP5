#include <sys/kmem.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "mcc_generated_files/mcc.h"
#include "memory.h"
#include "mem_locations.h"
#include "i2c_util.h"
#include "utils.h"

#include "usb_callbacks.h"

extern volatile CTRL_TRF_SETUP SetupPkt;

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

static struct chip_info chip_info = { };

/* Flash block(s) where the bootloader resides.*/
#define USER_REGION_BASE  APP_BASE
#define USER_REGION_TOP   (APP_BASE + APP_LENGTH)


/* Instruction sizes */
#ifdef __XC32__
	#define INSTRUCTIONS_PER_ROW 64
	#define BYTES_PER_INSTRUCTION 4
	#define WORDS_PER_INSTRUCTION 1
#else
	#error "Define instruction sizes for your platform"
#endif

#define BUFFER_LENGTH (INSTRUCTIONS_PER_ROW * BYTES_PER_INSTRUCTION)
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
/*datetime commands*/
#define WRITE_DATETIME 120
#define READ_DATETIME 121
/*UART commands*/
#define WRITE_UART 130
#define READ_UART 131

/* Data-to-program: buffer and attributes. */
static uint32_t write_address; /* program space word address */
static size_t write_length;    /* number of words, not bytes */
static uint8_t prog_buf[BUFFER_LENGTH];
static uint8_t i2c_on;
int usb_finished = 0;


static void clear_flash()
{

	uint32_t prog_addr = USER_REGION_BASE;
	int8_t res;

	while (prog_addr < USER_REGION_TOP) {
		erase_page((void*)prog_addr);
		if (res < 0)
			return;

		prog_addr += FLASH_BLOCK_SIZE;
	}

}

static void write_flash_row()
{
	/* Make sure a short buffer is padded with 0xff. */
	if (write_length < BUFFER_LENGTH)
		memset(prog_buf + write_length, 0xff,
		       BUFFER_LENGTH - write_length);
	write_row((void *)write_address, prog_buf);
}




static void write_i2c(uint8_t device_address, uint8_t* buffer, uint16_t len) {
	write_i2c_block(device_address,buffer,len);
}

void reset_cb(void)
{
    usb_finished=1;
}

static void empty_cb(void)
{
	/* Nothing to do here. */
}


static void write_data_cb(void)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in prog_buf[]. */
	write_flash_row();

}

static void write_i2c_cb(void)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in buf[]. */

    write_i2c(write_address,(char*)prog_buf,write_length);
}

static void write_display_cb(void)
{
	/* For OUT control transfers, data from the data stage of the request
	 * is in buf[]. */

    render_data_to_page(write_address>>16,write_address&0xFF,(char*)prog_buf,write_length);
}

static void write_datetime_cb(void) {
    RTCC_TimeSet(gmtime((time_t*)prog_buf));
}

static void write_uart_cb(void) {
    UART1_WriteBuffer(prog_buf, write_length);
}


void app_usb_reset_callback(void)
{

}



int8_t app_unknown_setup_request_callback()
{
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

	/* This handler handles request 254/dest=other/type=vendor only.*/
	if (SetupPkt.requestInfo.recipient == USB_SETUP_RECIPIENT_OTHER &&
	    SetupPkt.requestInfo.type == USB_SETUP_TYPE_VENDOR_BITFIELD &&
	    SetupPkt.requestInfo.direction == USB_SETUP_HOST_TO_DEVICE_BITFIELD ) {

		if (SetupPkt.bRequest == CLEAR_FLASH) {
			/* Clear flash Request */
			clear_flash();

			/* There will be NO data stage. This sends back the
			 * STATUS stage packet. */
            USBEP0SendRAMPtr(NULL, 0, USB_EP0_NO_DATA);
		}
		else if (SetupPkt.bRequest == SEND_DATA) {
			/* Write Data Request */
			if (SetupPkt.wLength > sizeof(prog_buf))
				return -1;

			write_address = SetupPkt.wValue | ((uint32_t) SetupPkt.wIndex) << 16;
			write_length = SetupPkt.wLength;

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
			if (SetupPkt.wLength > sizeof(prog_buf))
				return -1;

			memset(prog_buf, 0xff, sizeof(prog_buf));
            USBEP0Receive((char*)prog_buf, SetupPkt.wLength, &write_data_cb);
		}
		else if (SetupPkt.bRequest == SEND_RESET) {
			/* Reset to Application Request*/

			/* There will be NO data stage. This sends back the
			 * STATUS stage packet. */
            USBEP0SendRAMPtr(NULL, 0, USB_EP0_NO_DATA);
            reset_cb();
		}
		else if (SetupPkt.bRequest == WRITE_I2C_DATA) {
			write_address = SetupPkt.wValue;

			/* Check length */
			if (SetupPkt.wLength > sizeof(prog_buf))
				return -1;

			write_length = SetupPkt.wLength;
            USBEP0Receive((char*)prog_buf, SetupPkt.wLength, &write_i2c_cb);
		}
		else if (SetupPkt.bRequest == WRITE_DISPLAY) {
			write_address = SetupPkt.wValue | ((uint32_t) SetupPkt.wIndex) << 16;

			/* Check length */
			if (SetupPkt.wLength > sizeof(prog_buf))
				return -1;
			write_length = SetupPkt.wLength;
            USBEP0Receive((char*)prog_buf, SetupPkt.wLength, &write_display_cb);
            
		}
		else if (SetupPkt.bRequest == WRITE_DATETIME) {
			if (SetupPkt.wLength != sizeof(time_t))
				return -1;
            USBEP0Receive((char*)prog_buf, SetupPkt.wLength, &write_datetime_cb);			
		}

		else if (SetupPkt.bRequest == WRITE_UART) {
			if (SetupPkt.wLength > UART1_TransmitBufferSizeGet())
				return -1;
            USBEP0Receive((char*)prog_buf, SetupPkt.wLength, &write_uart_cb);			
		}
	}

	if (SetupPkt.requestInfo.recipient == USB_SETUP_RECIPIENT_OTHER &&
	    SetupPkt.requestInfo.type == USB_SETUP_TYPE_VENDOR_BITFIELD &&
	    SetupPkt.requestInfo.direction == USB_SETUP_DEVICE_TO_HOST_BITFIELD) {

		if (SetupPkt.bRequest == GET_CHIP_INFO) {
			/* Request Device Info Struct */
			chip_info.user_region_base = USER_REGION_BASE;
			chip_info.user_region_top = USER_REGION_TOP;
			chip_info.config_words_base = CONFIG_WORDS_BASE;
			chip_info.config_words_top = CONFIG_WORDS_TOP;

			chip_info.bytes_per_instruction = BYTES_PER_INSTRUCTION;
			chip_info.instructions_per_row = INSTRUCTIONS_PER_ROW;

            USBEP0SendRAMPtr((char*)&chip_info,	MIN(sizeof(struct chip_info), SetupPkt.wLength), USB_EP0_NO_OPTIONS);
		}

		if (SetupPkt.bRequest == REQUEST_DATA) {
			/* Request program data */
			uint32_t read_address;

			read_address = SetupPkt.wValue | ((uint32_t) SetupPkt.wIndex) << 16;

			/* Check length */
			if (SetupPkt.wLength > sizeof(prog_buf))
				return -1;

            USBEP0SendROMPtr((const uint8_t*)read_address, SetupPkt.wLength, USB_EP0_ROM);
		}
		else if (SetupPkt.bRequest == READ_I2C_DATA) {
			write_address = SetupPkt.wValue;
			if (SetupPkt.wLength > sizeof(prog_buf))
 				return -1;
			read_i2c_block(SetupPkt.wValue,(uint8_t*)prog_buf,SetupPkt.wLength);
            USBEP0SendRAMPtr((char*)prog_buf, SetupPkt.wLength, USB_EP0_NO_OPTIONS);
		}
		else if (SetupPkt.bRequest == READ_DATETIME) {
			if (SetupPkt.wLength != sizeof(time_t))
				return -1;
			RTCC_TimeGet((struct tm*)(&prog_buf[4]));
            ((time_t*)prog_buf)[0] = mktime((struct tm*)(&prog_buf[4]));
            USBEP0SendRAMPtr((char*)prog_buf, SetupPkt.wLength, USB_EP0_NO_OPTIONS);
		}
		else if (SetupPkt.bRequest == READ_UART) {
            write_length = UART1_ReadBuffer(prog_buf, SetupPkt.wLength);
            USBEP0SendRAMPtr((char*)prog_buf, write_length, USB_EP0_NO_OPTIONS);
		}
	}

	return 0; /* 0 = can handle this request. */
#undef MIN
}

