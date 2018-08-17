#include <xc.h>
#include "battery.h"
#include "mcc_generated_files/mcc.h"


enum BAT_STATUS get_bat_status(){
    
	if (PORTBbits.RB6) return CHARGING;
    return DISCHARGING;
}

#ifndef BOOTLOADER
double get_bat_charge(){
	double bat_voltage, bg_voltage;
    int i;
    ADC1_Initialize();
    ADC1_ChannelSelect(ADC1_BAT_SENSE);
    ADC1_Start();
    //Provide Delay
    for(i=0;i <1000;i++)
    {
    }
    ADC1_Stop();
    while(!ADC1_IsConversionComplete())
    {
        ADC1_Tasks();   
    }
    bat_voltage = ADC1_ConversionResultGet();
    ADC1_ChannelSelect(ADC1_CHANNEL_VBG);
    ADC1_Start();
    //Provide Delay
    for(i=0;i <1000;i++)
    {
    }
    ADC1_Stop();
    while(!ADC1_IsConversionComplete())
    {
        ADC1_Tasks();   
    }
    bg_voltage = ADC1_ConversionResultGet();
    
	return (bat_voltage/bg_voltage*1.2);
}
#endif