/**
  System Interrupts Generated Driver File 

  @Company:
    Microchip Technology Inc.

  @File Name:
    interrupt_manager.h

  @Summary:
    This is the generated driver implementation file for setting up the
    interrupts using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This source file provides implementations for PIC24 / dsPIC33 / PIC32MM MCUs interrupts.
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

/**
    Section: Includes
*/
#include <xc.h>

/**
    void INTERRUPT_Initialize (void)
*/
void INTERRUPT_Initialize (void)
{
    // Enable Multi Vector Configuration
    INTCONbits.MVEC = 1;
    
    //    MICI: I2C 1 Master
    //    Priority: 1
        IPC16bits.I2C1MIP = 1;
    //    Sub Priority: 0
        IPC16bits.I2C1MIS = 0;
    //    UERI: UART 2 Error
    //    Priority: 1
        IPC14bits.U2EIP = 1;
    //    Sub Priority: 0
        IPC14bits.U2EIS = 0;
    //    UTXI: UART 2 Transmission
    //    Priority: 1
        IPC14bits.U2TXIP = 1;
    //    Sub Priority: 0
        IPC14bits.U2TXIS = 0;
    //    URXI: UART 2 Reception
    //    Priority: 1
        IPC14bits.U2RXIP = 1;
    //    Sub Priority: 0
        IPC14bits.U2RXIS = 0;
    //    CNBI: PORT B Change Notification
    //    Priority: 7
        IPC2bits.CNBIP = 7;
    //    Sub Priority: 0
        IPC2bits.CNBIS = 0;
    //    CNCI: PORT C Change Notification
    //    Priority: 7
        IPC2bits.CNCIP = 7;
    //    Sub Priority: 0
        IPC2bits.CNCIS = 0;
    //    TI: Timer 2
    //    Priority: 1
        IPC4bits.T2IP = 1;
    //    Sub Priority: 0
        IPC4bits.T2IS = 0;
    //    UERI: UART 1 Error
    //    Priority: 1
        IPC13bits.U1EIP = 1;
    //    Sub Priority: 0
        IPC13bits.U1EIS = 0;
    //    UTXI: UART 1 Transmission
    //    Priority: 1
        IPC13bits.U1TXIP = 1;
    //    Sub Priority: 0
        IPC13bits.U1TXIS = 0;
    //    URXI: UART 1 Reception
    //    Priority: 1
        IPC13bits.U1RXIP = 1;
    //    Sub Priority: 0
        IPC13bits.U1RXIS = 0;
}
