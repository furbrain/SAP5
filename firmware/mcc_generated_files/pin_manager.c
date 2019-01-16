/**
  System Interrupts Generated Driver File 

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.c

  @Summary:
    This is the generated manager file for the MPLAB(c) Code Configurator device.  This manager
    configures the pins direction, initial state, analog setting.
    The peripheral pin select, PPS, configuration is also handled by this manager.

  @Description:
    This source file provides implementations for MPLAB(c) Code Configurator interrupts.
    Generation Information : 
        Product Revision  :  MPLAB(c) Code Configurator - 4.26.8
        Device            :  PIC32MM0256GPM028
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB             :  MPLAB X 3.60

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

*/


/**
    Section: Includes
*/
#include <xc.h>
#include "pin_manager.h"
#include "mcc.h"

/**
    void PIN_MANAGER_Initialize(void)
*/
void PIN_MANAGER_Initialize(void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATA = 0x0000;
    LATB = 0x4000;
    LATC = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISA = 0x001F;
    TRISB = 0xBF7F;
    TRISC = 0x0200;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPDA = 0x0000;
    CNPDB = 0x0040;
    CNPDC = 0x0000;
    CNPUA = 0x0000;
    CNPUB = 0x0000;
    CNPUC = 0x0200;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCA = 0x0000;
    ODCB = 0x0000;
    ODCC = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELA = 0x000F;
    ANSELB = 0x201C;


    /****************************************************************************
     * Interrupt On Change for group CNCONB - config
     ***************************************************************************/
	CNCONBbits.ON = 1; 
	CNCONBbits.CNSTYLE = 1; 

    /****************************************************************************
     * Interrupt On Change for group CNCONC - config
     ***************************************************************************/
	CNCONCbits.ON = 1; 
	CNCONCbits.CNSTYLE = 1; 

    /****************************************************************************
     * Interrupt On Change for group CNEN0B - positive
     ***************************************************************************/
	CNEN0Bbits.CNIE0B6 = 1; // Pin : RB6

    /****************************************************************************
     * Interrupt On Change for group CNEN0C - positive
     ***************************************************************************/
	CNEN0Cbits.CNIE0C9 = 0; // Pin : RC9

    /****************************************************************************
     * Interrupt On Change for group CNEN1B - negative
     ***************************************************************************/
	CNEN1Bbits.CNIE1B6 = 0; // Pin : RB6

    /****************************************************************************
     * Interrupt On Change for group CNEN1C - negative
     ***************************************************************************/
	CNEN1Cbits.CNIE1C9 = 1; // Pin : RC9

    /****************************************************************************
     * Interrupt On Change for group CNFB - flag
     ***************************************************************************/
	CNFBbits.CNFB6 = 0; // Pin : RB6

    /****************************************************************************
     * Interrupt On Change for group CNFC - flag
     ***************************************************************************/
	CNFCbits.CNFC9 = 0; // Pin : RC9

    IEC0bits.CNBIE = 1; // Enable CNBI interrupt 
    IEC0bits.CNCIE = 1; // Enable CNCI interrupt 
}

/* Interrupt service routine for the CNBI interrupt. */
void __attribute__ ((vector(_CHANGE_NOTICE_B_VECTOR), interrupt(IPL7SOFT))) _CHANGE_NOTICE_B( void )
{
    if(IFS0bits.CNBIF == 1)
    {
        // Clear the flag
        IFS0CLR= 1 << _IFS0_CNBIF_POSITION; //Clear IFS0bits.CNBIF
        // interrupt on change for group CNFB
        if(CNFBbits.CNFB6 == 1)
        {
            CNFBCLR = 0x40;  //Clear CNFBbits.CNFB6
            // Add handler code here for Pin - RB6
        }
    }
}
/* Interrupt service routine for the CNCI interrupt. */
void __attribute__ ((vector(_CHANGE_NOTICE_C_VECTOR), interrupt(IPL7SOFT))) _CHANGE_NOTICE_C( void )
{
    if(IFS0bits.CNCIF == 1)
    {
        // Clear the flag
        IFS0CLR= 1 << _IFS0_CNCIF_POSITION; //Clear IFS0bits.CNCIF
        // interrupt on change for group CNFC
        if(CNFCbits.CNFC9 == 1)
        {
            CNFCCLR = 0x200;  //Clear CNFCbits.CNFC9
            // Add handler code here for Pin - RC9
        }
    }
}
