/* 
 * File:   mem_locations.h
 * Author: phil
 *
 * Created on 04 August 2018, 11:00
 */
#ifndef MEM_LOCATIONS_H
#define	MEM_LOCATIONS_H
/* Definitions used for bootloader */
#define SPECIAL 0x3000 /* includes boot mem, exception mem, debug exec mem */
#define BL_LENGTH 0x4000 /* Length of bootloader's main flash component */

/* Variables to pass into the bootloader. All addresses are virtual.
   These represent the base and length of the user or application
   flash memory. */
#define APP_BASE  (0x9D000000 + BL_LENGTH)
#define APP_LENGTH  (0x40000 - BL_LENGTH)
#define FLASH_BLOCK_SIZE 0x0800 /* From the datasheet */
#define CONFIG_WORDS_BASE 0x1fc01740
#define CONFIG_WORDS_TOP  0x1fc017F0

#endif	/* MEM_LOCATIONS_H */
