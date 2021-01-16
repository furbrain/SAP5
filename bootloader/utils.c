#include <xc.h>
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/rtcc.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/pin_manager.h"
#include "utils.h"
#include "app_type.h"
#ifndef BOOTLOADER
#include "exception.h"
#include "beep.h"
#include "bt.h"
#else
#define THROW_WITH_REASON(reason, code) {}
#define beep_initialised false
#define beep_finish() {}
#endif



#ifndef BOOTLOADER

static bool beep_initialised = false;


static
bool check_if_bt(void) {
    TRISBbits.TRISB0 = 1;
    CNPDBbits.CNPDB0 = 1;
    delay_ms_safe(3);
    if (PORTBbits.RB0==1) {
        CNPDBbits.CNPDB0 = 0;
        return true;
    }
    CNPDBbits.CNPDB0 = 0;
    return false;
}

static
void pwm_beep_init(void) {
    // ON disabled; MOD 16-Bit Single Edge, Toggle; ALTSYNC disabled; SIDL disabled; OPS Each Time Base Period Match; CCPSLP disabled; TMRSYNC disabled; RTRGEN disabled; CCSEL disabled; ONESHOT disabled; TRIGEN disabled; T32 16 Bit; SYNC None; OPSSRC Timer Interrupt Event; TMRPS 1:4; CLKSEL SYSCLK; 
    CCP2CON1 = 0x43;
    //OCCEN enabled; OCDEN enabled; ASDGM disabled; OCEEN disabled; ICGSM Level-Sensitive mode; OCFEN disabled; ICS ICM2; SSDG disabled; AUXOUT Disabled; ASDG None; OCAEN disabled; OCBEN disabled; OENSYNC disabled; PWMRSEN disabled; 
    CCP2CON2 = 0xC000000;
    //DT 0; OETRIG disabled; OSCNT None; POLACE disabled; POLBDF disabled; PSSBDF Inactive; OUTM Half bridge output; PSSACE Inactive; 
    CCP2CON3 = 0x20A0000;
    //SCEVT disabled; TRSET disabled; ICOV disabled; ASEVT disabled; ICGARM disabled; RBWIP disabled; TRCLR disabled; RAWIP disabled; TMRHWIP disabled; TMRLWIP disabled; PRLWIP disabled; 
    CCP2STAT = 0x0;
    TRISBCLR = 0x03;    
}

/* initialise beeping circuitry...*/
void bt_and_beep_initialise(void){
    bt_present = check_if_bt();
    if (bt_present) {
        bt_init();
    } else {
        pwm_beep_init();
    }
    beep_initialised = true;
}

static
void pwm_beep_start(double freq) {
    uint32_t priVal;
    priVal = (BEEP_FREQ/ (freq*2));
    //priVal = (2 * 0x10000 * freq) / BEEP_FREQ;
    CCP2PR = priVal;
    CCP2RA = priVal;
    CCP2CON1bits.ON = 1;    
}

static
void pwm_beep_stop(void) {
    CCP2CON1bits.ON = 0;
}
/* beep at freq for duration milliseconds*/
void beep(double freq, int duration) {
    if (!beep_initialised) {
        THROW_WITH_REASON("Beep not initialised", ERROR_HARDWARE_NOT_IDENTIFIED);
    }
    if (bt_present) {
        bt_beep_start(freq);
    } else {
        pwm_beep_start(freq);
    }
    delay_ms_safe(duration);
    if (bt_present) {
        bt_beep_stop();
    } else {
        pwm_beep_stop();
    }
}

#endif

void delay_ms(int count) {
    while (count > 0) {
        TMR1_Counter16BitSet(0);
        TMR1_Start();
        while (!TMR1_GetElapsedThenClear()) {
            TMR1_Tasks_16BitOperation();
        }
        count--;
    }
}

void delay_ms_safe(int count) {
    while (count > 500) {
        wdt_clear();
        delay_ms(500);
        count -=500;
    }
    wdt_clear();
    delay_ms(count);
    wdt_clear();
}

struct tm get_time_buffer;
/* get current time in time_t format */
time_t utils_get_time() {
    RTCC_TimeGet(&get_time_buffer);
    return (int32_t)mktime(&get_time_buffer);
}


void wdt_clear(void){
    volatile unsigned short *wdtKey;
    wdtKey = (unsigned short*) (&WDTCON)+1;
    *wdtKey = 0x5743;
}


void sys_reset(void){
    /* The following reset procedure is from the Family Reference Manual,
	 * Chapter 7, "Resets," under "Software Resets." */
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    int x;
#pragma GCC diagnostic pop
    INTERRUPT_GlobalDisable();
    IFS0CLR = 0xffffffff;
    IFS1CLR = 0xffffffff;
    IFS2CLR = 0xffffffff;
    IFS3CLR = 0xffffffff;
	/* Unlock sequence */
	SYSKEY = 0x00000000;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;

	/* Set the reset bit and then read it to trigger the reset. */
	RSWRSTSET = 1;
	x = RSWRST;

	/* Required NOP instructions */
	asm("nop\n nop\n nop\n nop\n");
}

void utils_turn_off(int32_t a) {
    //turn off peripherals
    if (beep_initialised) {
        beep_finish();        
    }
    PERIPH_EN_SetLow();
    while (!SWITCH_GetValue()) {
        delay_ms_safe(1);
    }
    sys_reset();
}


int utils_flash_memory (void *dest, const void *data, enum FLASH_OP op) {
    // Fill out relevant registers
    NVMADDR = (uint32_t)dest;
    switch (op) {
        case FLASH_WRITE_DWORD:
            NVMDATA0 = *((uint32_t*)data);
            NVMDATA1 = *(((uint32_t*)data)+1);
            break;
        case FLASH_WRITE_ROW:
        case FLASH_ERASE_PAGE:
            NVMSRCADDR = (uint32_t)data;
            break;
        case FLASH_ERASE_CHIP:
        case FLASH_CLEAR_ERROR:
            break;
        default:
            THROW_WITH_REASON("Invalid flash operation",ERROR_FLASH_STORE_FAILED);
    }
    // Suspend or Disable all Interrupts
    INTERRUPT_GlobalDisable();
    // Enable Flash Write/Erase Operations and Select
    // Flash operation to perform
    NVMCON = op;
    // Write Keys
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    // Start the operation using the Set Register
    NVMCONSET = 0x8000;
    // Wait for operation to complete
    while (NVMCON & 0x8000);
    // Restore Interrupts
    INTERRUPT_GlobalEnable();
    // Disable NVM write enable
    NVMCONCLR = 0x0004000;
    // Return WRERR and LVDERR Error Status Bits
    return (NVMCON & 0x3000);
}


