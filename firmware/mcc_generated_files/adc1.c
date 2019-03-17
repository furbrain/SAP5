
/**
  ADC1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    adc1.c

  @Summary
    This is the generated header file for the ADC1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for ADC1.
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
#include "adc1.h"

/**
  Section: Data Type Definitions
*/

/* ADC Driver Hardware Instance Object

  @Summary
    Defines the object required for the maintenance of the hardware instance.

  @Description
    This defines the object required for the maintenance of the hardware
    instance. This object exists once per hardware instance of the peripheral.

 */
typedef struct
{
	uint8_t intSample;
}

ADC_OBJECT;

static ADC_OBJECT adc1_obj;

/**
  Section: Driver Interface
*/


void ADC1_Initialize (void)
{
    // ASAM disabled; DONE disabled; FORM Integer 16-bit; SAMP disabled; SSRC Clearing sample bit ends sampling and starts conversion; SIDL disabled; MODE12 12-bit; ON enabled; 

   AD1CON1 = 0x8008;

    // CSCNA disabled; BUFM disabled; SMPI 1; OFFCAL disabled; VCFG AVDD/AVSS; BUFREGEN disabled; 

   AD1CON2 = 0x0;

    // SAMC 25; EXTSAM disabled; ADRC PBCLK; ADCS 29; 

   AD1CON3 = 0x191D;

    // CH0SA AN0; CH0NA AVSS; 

   AD1CHS = 0x0;

    // CSS9 disabled; CSS8 disabled; CSS7 disabled; CSS6 disabled; CSS5 disabled; CSS4 disabled; CSS3 disabled; CSS2 disabled; CSS1 disabled; CSS0 disabled; CSS11 disabled; CSS10 disabled; CSS30 disabled; CSS29 disabled; CSS28 disabled; CSS27 disabled; 

   AD1CSS = 0x0;

    // CHH2 disabled; CHH1 disabled; CHH0 disabled; CHH11 enabled; CHH10 disabled; CHH9 disabled; CHH8 disabled; CHH7 disabled; CHH6 disabled; CHH5 disabled; CHH4 disabled; CHH3 disabled; 

   AD1CHIT = 0x800;


   adc1_obj.intSample = AD1CON2bits.SMPI;
   
}

void ADC1_Start(void)
{
   AD1CON1SET = (1 << _AD1CON1_SAMP_POSITION);
}
void ADC1_Stop(void)
{
   AD1CON1CLR = (1 << _AD1CON1_SAMP_POSITION);
}
uint16_t ADC1_ConversionResultBufferGet(uint16_t *buffer)
{
    int count;
    uint16_t *ADC16Ptr;

    ADC16Ptr = (uint16_t *)&(ADC1BUF0);

    for(count=0;count<=adc1_obj.intSample;count++)
    {
        buffer[count] = (uint16_t)*ADC16Ptr;
        ADC16Ptr++;
    }
    return count;
}
uint16_t ADC1_ConversionResultGet(void)
{
    return ADC1BUF0;
}
bool ADC1_IsConversionComplete( void )
{
    return AD1CON1bits.DONE; //Wait for conversion to complete   
}
void ADC1_ChannelSelect( ADC1_CHANNEL channel )
{
    AD1CHS = channel;
}


void ADC1_Tasks ( void )
{
    // clear the ADC interrupt flag
    IFS1CLR= 1 << _IFS1_AD1IF_POSITION;
}


/**
  End of File
*/
