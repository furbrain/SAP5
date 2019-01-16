
/**
  MCCP2 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    mccp2.c

  @Summary
    This is the generated header file for the MCCP2 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for MCCP2.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - pic24-dspic-pic32mm : v1.35
        Device            :  PIC32MM0256GPM028
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB 	          :  MPLAB X 3.60
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

/**
  Section: Included Files
*/

#include <xc.h>
#include "mccp2_compare.h"

/** OC Mode.

  @Summary
    Defines the OC Mode.

  @Description
    This data type defines the OC Mode of operation.

*/

static uint16_t         gMCCP2Mode;

/**
  Section: Driver Interface
*/


void MCCP2_COMPARE_Initialize (void)
{
    // ON enabled; MOD Variable Frequency Pulse; ALTSYNC disabled; SIDL disabled; OPS Each Time Base Period Match; CCPSLP disabled; TMRSYNC disabled; RTRGEN disabled; CCSEL disabled; ONESHOT disabled; TRIGEN disabled; T32 16 Bit; SYNC None; OPSSRC Timer Interrupt Event; TMRPS 1:64; CLKSEL SYSCLK; 
    CCP2CON1 = (0x80C7 & 0xFFFF7FFF); //Disabling CCPON bit
    //OCCEN enabled; OCDEN enabled; ASDGM disabled; OCEEN disabled; ICGSM Level-Sensitive mode; OCFEN disabled; ICS ICM2; SSDG disabled; AUXOUT Disabled; ASDG None; OCAEN disabled; OCBEN disabled; OENSYNC disabled; PWMRSEN disabled; 
    CCP2CON2 = 0xC000000;
    //DT 0; OETRIG disabled; OSCNT None; POLACE disabled; POLBDF disabled; PSSBDF Tri-state; OUTM Half bridge output; PSSACE Tri-state; 
    CCP2CON3 = 0x2000000;
    //SCEVT disabled; TRSET disabled; ICOV disabled; ASEVT disabled; ICGARM disabled; RBWIP disabled; TRCLR disabled; RAWIP disabled; TMRHWIP disabled; TMRLWIP disabled; PRLWIP disabled; 
    CCP2STAT = 0x0;
    //TMRL 0; TMRH 0; 
    CCP2TMR = 0x0;
    //PRH 0; PRL 0; 
    CCP2PR = 0x0;
    //CMPA 1398; 
    CCP2RA = 0x576;
    //CMPB 0; 
    CCP2RB = 0x0;
    //BUFL 0; BUFH 0; 
    CCP2BUF = 0x0;

    CCP2CON1bits.ON = 0x1; //Enabling CCP

    gMCCP2Mode = CCP2CON1bits.MOD;

}

void MCCP2_COMPARE_Tasks( void )
{
    if(IFS2bits.CCP2IF)
    {
        IFS2CLR= 1 << _IFS2_CCP2IF_POSITION;
    }
}


void MCCP2_COMPARE_TimerTasks( void )
{
    if(IFS2bits.CCT2IF)
    {
        IFS2CLR= 1 << _IFS2_CCT2IF_POSITION;
    }
}

void MCCP2_COMPARE_Start( void )
{
    /* Start the Timer */
    CCP2CON1SET = (1 << _CCP2CON1_ON_POSITION);
}
void MCCP2_COMPARE_Stop( void )
{
    /* Start the Timer */
    CCP2CON1CLR = (1 << _CCP2CON1_ON_POSITION);
}

void MCCP2_COMPARE_SingleCompare16ValueSet( uint16_t value )
{   
    CCP2RA = value;
}


void MCCP2_COMPARE_DualCompareValueSet( uint16_t priVal, uint16_t secVal )
{
    
    CCP2RA = priVal;
	
    CCP2RB = secVal;
}
void MCCP2_COMPARE_DualEdgeBufferedConfig( uint16_t priVal, uint16_t secVal )
{
    
    CCP2RA = priVal;
	
    CCP2RB = secVal;
}
void MCCP2_COMPARE_CenterAlignedPWMConfig( uint16_t priVal, uint16_t secVal )
{
    
    CCP2RA = priVal;
	
    CCP2RB = secVal;
}
void MCCP2_COMPARE_VariableFrequencyPulseConfig( uint16_t priVal )
{
    CCP2RA = priVal;
	
}
bool MCCP2_COMPARE_IsCompareCycleComplete( void )
{
    return(IFS2bits.CCP2IF);
}
bool MCCP2_COMPARE_TriggerStatusGet( void )
{
    return( CCP2STATbits.CCPTRIG );
    
}
void MCCP2_COMPARE_TriggerStatusSet( void )
{
    CCP2STATSET = (1 << _CCP2STAT_TRSET_POSITION);
}
void MCCP2_COMPARE_TriggerStatusClear( void )
{
    /* Clears the trigger status */
    CCP2STATCLR = (1 << _CCP2STAT_TRCLR_POSITION);
}
bool MCCP2_COMPARE_SingleCompareStatusGet( void )
{
    return( CCP2STATbits.SCEVT );
}
void MCCP2_COMPARE_SingleCompareStatusClear( void )
{
    /* Clears the trigger status */
    CCP2STATCLR = (1 << _CCP2STAT_SCEVT_POSITION);
    
}
/**
 End of File
*/
