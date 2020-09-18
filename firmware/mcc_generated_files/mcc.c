


/**
  @Generated PIC24 / dsPIC33 / PIC32MM MCUs Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    mcc.c

  @Summary:
    This is the mcc.c file generated using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - pic24-dspic-pic32mm : v1.35
        Device            :  PIC32MM0256GPM028
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB             :  MPLAB X 3.60
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/


// Configuration bits: selected in the GUI

// FDEVOPT
#pragma config SOSCHP = OFF    // Secondary Oscillator High Power Enable bit->SOSC oprerates in normal power mode.
#pragma config ALTI2C = OFF    // Alternate I2C1 Pins Location Enable bit->Primary I2C1 pins are used
#pragma config FUSBIDIO = ON    // USBID pin control->USBID pin is controlled by the port function
#pragma config FVBUSIO = OFF   // VBUS Pin Control->VBUS pin is controlled by the port function

// FICD
#pragma config JTAGEN = OFF    // JTAG Enable bit->JTAG is disabled
#pragma config ICS = PGx1    // ICE/ICD Communication Channel Selection bits->Communicate on PGEC1/PGED1

// FPOR
#pragma config BOREN = BOR3    // Brown-out Reset Enable bits->Brown-out Reset enabled in hardware; SBOREN bit disabled
#pragma config RETVR = OFF    // Retention Voltage Regulator Enable bit->Retention regulator is disabled
#pragma config LPBOREN = ON    // Downside Voltage Protection Enable bit->Low power BOR is enabled, when main BOR is disabled

// FWDT
#pragma config SWDTPS = PS131072   // Sleep Mode Watchdog Timer Postscale Selection bits->1:1048576
#pragma config FWDTWINSZ = PS25_0    // Watchdog Timer Window Size bits->Watchdog timer window size is 25%
#pragma config WINDIS = OFF    // Windowed Watchdog Timer Disable bit->Watchdog timer is in non-window mode
#pragma config RWDTPS = PS4096    // Run Mode Watchdog Timer Postscale Selection bits->1:4096
#pragma config RCLKSEL = LPRC    // Run Mode Watchdog Timer Clock Source Selection bits->Clock source is LPRC (same as for sleep mode)
#pragma config FWDTEN = ON    // Watchdog Timer Enable bit->WDT is enabled

// FOSCSEL
#pragma config FNOSC = FRCDIV    // Oscillator Selection bits->Fast RC oscillator (FRC) with divide-by-N
#pragma config PLLSRC = FRC    // System PLL Input Clock Selection bit->FRC oscillator is selected as PLL reference input on device reset
#pragma config SOSCEN = ON    // Secondary Oscillator Enable bit->Secondary oscillator (SOSC) is disabled
#pragma config IESO = OFF    // Two Speed Startup Enable bit->Two speed startup is disabled
#pragma config POSCMOD = OFF    // Primary Oscillator Selection bit->Primary oscillator is disabled
#pragma config OSCIOFNC = OFF    // System Clock on CLKO Pin Enable bit->OSCO pin operates as a normal I/O
#pragma config SOSCSEL = ON    // Secondary Oscillator External Clock Enable bit->External clock is connected to SOSCO pin (RA4 and RB4 are controlled by I/O port registers)
#pragma config FCKSM = CSECMD    // Clock Switching and Fail-Safe Clock Monitor Enable bits->Clock switching is enabled; Fail-safe clock monitor is disabled

// FSEC
#pragma config CP = OFF    // Code Protection Enable bit->Code protection is disabled

#include "mcc.h"

void SYSTEM_Initialize(void)
{
    PIN_MANAGER_Initialize();
    INTERRUPT_Initialize();
    OSCILLATOR_Initialize();
    UART2_Initialize();
    I2C1_Initialize();
    UART1_Initialize();
    ADC1_Initialize();
    TMR2_Initialize();
    //RTCC_Initialize();
    TMR1_Initialize();
    INTERRUPT_GlobalEnable();
}


void OSCILLATOR_Initialize(void)
{
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
    // CF No Clock Failure; FRCDIV FRC/1; SLPEN Device will enter Idle mode when a WAIT instruction is issued; NOSC SPLL; SOSCEN disabled; CLKLOCK Clock and PLL selections are locked; OSWEN Oscillator switch initiate; 
    OSCCON = (0x180 | _OSCCON_OSWEN_MASK);
    SYSTEM_RegLock();
    // ON disabled; DIVSWEN disabled; RSLP disabled; ROSEL SYSCLK; OE disabled; SIDL disabled; RODIV 0; 
    REFO1CON = 0x0;
    // ROTRIM 0; 
    REFO1TRIM = 0x0;
}

/**
 End of File
*/