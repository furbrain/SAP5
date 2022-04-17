#include <xc.h>
#include <string.h>
#include <stdbool.h>

#include "mcc_generated_files/mcc.h"
#include "app_type.h"
#include "mem_locations.h"

#include "battery.h"
#include "i2c_util.h"
#include "display.h"
#include "usb_callbacks.h"
#include "utils.h"
#include "memory.h"
#include "version.h"
#include "mcc_generated_files/usb/usb_hal_pic32mm.h"

#define DISPLAY_ADDRESS 0x3C

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
	render_data_to_screen(0,104,bat_status,24);
	for (i = 0; i< 24; ++i) {
		bat_status[i] = reverse(bat_status[i]);
	}
	render_data_to_screen(1,104,bat_status,24);
}

void disable_modules(void) {
    SYSTEM_RegUnlock();
    PMDCONbits.PMDLOCK = 0;
    PMD1 = 0xffffffff;
    PMD2 = 0xffffffff;
    PMD3 = 0xffffffff;
    PMD4 = 0xffffffff;
    PMD5 = 0xffffffff;
    PMD6 = 0xfffffffe; //leave RTCC on...
    PMD7 = 0xffffffff;
    PMDCONbits.PMDLOCK = 1;
    PWRCONbits.VREGS = 0;
    PWRCONbits.RETEN = 1;    
    OSCCONbits.SLPEN = 1;
    SYSTEM_RegLock();    
}

void enable_modules(void) {
    SYSTEM_RegUnlock();
    PMDCONbits.PMDLOCK = 0;
    PMD1 = 0x0;
    PMD2 = 0x0;
    PMD3 = 0x0;
    PMD4 = 0x0;
    PMD5 = 0x0;
    PMD6 = 0x0;
    PMD7 = 0x0;
    PMDCONbits.PMDLOCK = 1;
    SYSTEM_RegLock();
}

void sleep(void) {
    TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    TRISAbits.TRISA4 = 1;
    TRISCbits.TRISC9 = 1;
    TRISBbits.TRISB6 = 1;
    LATA = 0;
    LATB = 0;
    LATC = 0;
    INTCONbits.MVEC = 1;
    IPC2bits.CNBIP = 7;
    IPC2bits.CNBIS = 0;
    IPC2bits.CNCIP = 7;
    IPC2bits.CNCIS = 0;
    disable_modules();
    __builtin_enable_interrupts();
    asm("wait");
    asm("nop;nop;nop;nop;");
    enable_modules();
}

void JumpToApp(void)
{       
        void (*fptr)(void);
        INTERRUPT_GlobalDisable();
        DMACONbits.ON = 0;
        fptr = (void (*)(void))APP_BASE+1;
        fptr();
}       


void run_usb(void) {
    int counter = 0;
    //Re-enable peripherals...
    
	enum BAT_STATUS bat_status;
    SYSTEM_Initialize();
    memory_clear_errors();
	/* setup ports */	
/* enable peripherals */
    PERIPH_EN_SetHigh();
    TMR2_Start();
    delay_ms(100);
	display_init();
	display_clear(true);
	delay_ms(3);
	while (!usb_finished) {
		bat_status = battery_get_status();
		if (bat_status==DISCHARGING) {
            U1PWRCbits.USBPWR = 0;
            sys_reset(0);
        }
		counter++;
		if ((counter & 0xffff)==0) {
			// only update display every 32 cycles
			if (bat_status==CHARGING) display_show_bat(-1);
			if (bat_status==CHARGED) display_show_bat(18);
		}
        wdt_clear();
	}
    USBDeviceDetach();
    delay_ms(80);
    JumpToApp();
}

void low_power_oscillator_initialise(void) {
    SYSTEM_RegUnlock();
    // ORPOL disabled; SIDL disabled; SRC USB; TUN Center frequency; POL disabled; ON disabled; 
    OSCTUN = 0x1000;
    // PLLODIV 1:4; PLLMULT 12x; PLLICLK FRC; 
    SPLLCON = 0x2050080;
    // WDTO disabled; GNMI disabled; CF disabled; WDTS disabled; NMICNT 0; LVD disabled; SWNMI disabled; 
    RNMICON = 0x0;
    // SBOREN disabled; VREGS disabled; RETEN disabled; 
    PWRCON = 0x0;
    //Clear NOSC,CLKLOCK and OSWEN bits
    OSCCONCLR = _OSCCON_NOSC_MASK | _OSCCON_CLKLOCK_MASK | _OSCCON_OSWEN_MASK;
    // CF No Clock Failure; FRCDIV FRC/1; SLPEN Device will enter Idle mode when a WAIT instruction is issued; NOSC SPLL; SOSCEN enabled; CLKLOCK Clock and PLL selections are locked; OSWEN Oscillator switch initiate; 
#ifdef EXTERNAL_CLOCK
    OSCCON = (0x002 | _OSCCON_OSWEN_MASK);
#else
    OSCCON = (0x000 | _OSCCON_OSWEN_MASK);
#endif    
    SYSTEM_RegLock();
    // ON disabled; DIVSWEN disabled; RSLP disabled; ROSEL SYSCLK; OE disabled; SIDL disabled; RODIV 0; 
    REFO1CON = 0x0;
    // ROTRIM 0; 
    REFO1TRIM = 0x0;

}

void initialise(void) {
    RCON = 0x0;
    enable_modules();
    PIN_MANAGER_Initialize();
    INTERRUPT_Initialize();
    low_power_oscillator_initialise();
    //I2C1_Initialize();
    //USBDeviceInit();
    //UART1_Initialize();
    //ADC1_Initialize();
    TMR2_Initialize();
    TMR1_Initialize();
    RTCC_TimeReset(true);
    RTCC_Initialize();
    //USBDeviceAttach();
    INTERRUPT_GlobalEnable();
    PERIPH_EN_SetLow();
    
}

int main(void)
{
    initialise();
    delay_ms_safe(20);
    while(1) {
        enable_modules();
        if (PORTBbits.RB6) {
            /* USB connected */
            PERIPH_EN_SetHigh();
            run_usb();
        }
        if (!SWITCH_GetValue()) {
            /* button has been pressed*/
            RCON = 0x0;
            JumpToApp();
        }
        delay_ms_safe(40);
        PIN_MANAGER_Initialize();
        PERIPH_EN_SetLow();
        sleep();
        sys_reset(0);
    }
}

