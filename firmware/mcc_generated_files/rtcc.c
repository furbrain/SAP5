
/**
  RTCC Generated Driver API Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    rtcc.c

  @Summary:
    This is the generated header file for the RTCC driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides APIs for driver for RTCC.
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
#include "rtcc.h"
#include "mcc.h"

/**
  Section: Static function
*/


static bool rtccTimeInitialized;
static bool RTCCTimeInitialized(void);
static uint8_t ConvertHexToBCD(uint8_t hexvalue);
static uint8_t ConvertBCDToHex(uint8_t bcdvalue);

/**
   Section: Driver Interface Function Definitions
*/

void RTCC_Initialize(void)
{
  
   SYSTEM_RegUnlock();
   RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
   RTCCON1CLR = (1 << _RTCCON1_ON_POSITION);
     
   if(!RTCCTimeInitialized())
   {
       // set 2018-05-05 10-56-41
       RTCDATE = 0x18050506; // Year/Month/Date/Wday
       RTCTIME = 0x10564100; //  hours/minutes/seconds
       
   }
       // ON enabled; OUTSEL Alarm Event; WRLOCK disabled; AMASK Every Half Second; ALMRPT 0; RTCOE disabled; CHIME disabled; ALRMEN disabled; 
       RTCCON1 = 0x8000;
   
       // DIV 15999; CLKSEL LPRC; FDIV 0; 
       RTCCON2 = 0x3E7F0001;
   RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);
   SYSTEM_RegLock();

}

bool RTCC_TimeGet(struct tm *currentTime)
{
    uint32_t register_value;
    if(RTCSTATbits.SYNC){
        return false;
    }

    SYSTEM_RegUnlock();
    RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
     
    register_value = RTCDATE;
    currentTime->tm_year = ConvertBCDToHex((register_value & 0xFF000000) >> 24);
    currentTime->tm_mon = ConvertBCDToHex((register_value & 0xFF0000) >> 16);
    currentTime->tm_mday = ConvertBCDToHex((register_value & 0xFF00) >> 8);
    currentTime->tm_wday = ConvertBCDToHex(register_value & 0xF);
    
    register_value = RTCTIME;
    currentTime->tm_hour = ConvertBCDToHex((register_value & 0xFF000000) >> 24);
    currentTime->tm_min = ConvertBCDToHex((register_value & 0xFF0000) >> 16);
    currentTime->tm_sec = ConvertBCDToHex((register_value & 0xFF00) >> 8);
   
    RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);
    SYSTEM_RegLock();

    return true;
}

void RTCC_TimeSet(struct tm *initialTime)
{
   SYSTEM_RegUnlock();
   RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
   RTCCON1CLR = (1 << _RTCCON1_ON_POSITION);
  

   // set RTCC initial time
   RTCDATE = (ConvertHexToBCD(initialTime->tm_year) << 24) | (ConvertHexToBCD(initialTime->tm_mon) << 16 ) | (ConvertHexToBCD(initialTime->tm_mday) << 8) | ConvertHexToBCD(initialTime->tm_wday) ; // YEAR/MONTH-1/DAY-1/WEEKDAY
   RTCTIME = (ConvertHexToBCD(initialTime->tm_hour) << 24) | (ConvertHexToBCD(initialTime->tm_min) << 16 ) | (ConvertHexToBCD(initialTime->tm_sec) << 8) ; // HOURS/MINUTES/SECOND
          
   // Enable RTCC 
   RTCCON1SET = (1 << _RTCCON1_ON_POSITION);
    
    // SET WRLOCK
   RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);
   SYSTEM_RegLock();

}

bool RTCC_BCDTimeGet(bcdTime_t *currentTime)
{
    uint32_t register_value;
    if(RTCSTATbits.SYNC){
        return false;
    }

    SYSTEM_RegUnlock();
    RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
    
    register_value = RTCDATE;
    currentTime->tm_year = (register_value & 0xFF000000) >> 24;
    currentTime->tm_mon = (register_value & 0xFF0000) >> 16;
    currentTime->tm_mday = (register_value & 0xFF00) >> 8;
    currentTime->tm_wday = (register_value & 0xF);
    
    register_value = RTCTIME;
    currentTime->tm_hour = (register_value & 0xFF000000) >> 24;
    currentTime->tm_min = (register_value & 0xFF0000) >> 16;
    currentTime->tm_sec = (register_value & 0xFF00) >> 8;
   
    RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);
    SYSTEM_RegLock();

    return true;
}

void RTCC_BCDTimeSet(bcdTime_t *initialTime)
{
   SYSTEM_RegUnlock();
   RTCCON1CLR = (1 << _RTCCON1_WRLOCK_POSITION);
   RTCCON1CLR = (1 << _RTCCON1_ON_POSITION);

   // set RTCC initial time
   RTCDATE = (initialTime->tm_year << 24) | (initialTime->tm_mon << 16 ) | (initialTime->tm_mday << 8) | (initialTime->tm_wday) ; // YEAR/MONTH-1/DAY-1/WEEKDAY
   RTCTIME = (initialTime->tm_hour << 24) | (initialTime->tm_min << 16 ) | (initialTime->tm_sec << 8) ; // HOURS/MINUTES/SECOND
              
   // Enable RTCC  
   RTCCON1SET = (1 << _RTCCON1_ON_POSITION);
   
   // SET WRLOCK 
   RTCCON1SET = (1 << _RTCCON1_WRLOCK_POSITION);
   SYSTEM_RegLock();

}

/**
 This function implements RTCC_TimeReset.This function is used to
 used by application to reset the RTCC value and reinitialize RTCC value.
*/
void RTCC_TimeReset(bool reset)
{
    rtccTimeInitialized = reset;
}

static bool RTCCTimeInitialized(void)
{
    return(rtccTimeInitialized);
}

static uint8_t ConvertHexToBCD(uint8_t hexvalue)
{
    uint8_t bcdvalue;
    bcdvalue = (hexvalue / 10) << 4;
    bcdvalue = bcdvalue | (hexvalue % 10);
    return (bcdvalue);
}

static uint8_t ConvertBCDToHex(uint8_t bcdvalue)
{
    uint8_t hexvalue;
    hexvalue = (((bcdvalue & 0xF0) >> 4)* 10) + (bcdvalue & 0x0F);
    return hexvalue;
}


/* Function:
    bool RTCC_Task(void)

  Summary:
    Status function which returns the RTCC interrupt flag status

  Description:
    This is the status function for the RTCC peripheral. 
*/
bool RTCC_Task(void)
{
    bool status;
    status = IFS1bits.RTCCIF;
    if( IFS1bits.RTCCIF)
    {
        IFS1CLR= 1 << _IFS1_RTCCIF_POSITION;
    }
    return status;
}
/**
 End of File
*/
