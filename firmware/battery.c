#include <xc.h>
#include <stdfix.h>
#include "battery.h"
#include "mcc_generated_files/mcc.h"


enum BAT_STATUS get_bat_status(){
    
	if (PORTBbits.RB6) return CHARGING;
    return DISCHARGING;
}

#ifndef BOOTLOADER
accum get_bat_charge(){
	accum bat_voltage, bg_voltage;
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

int battery_get_units() {
    accum voltage;
    int result;
    voltage = get_bat_charge();
    /* scale voltage to range of 0->1, where 0 is minimum acceptable and 1 is maximum acceptable */
    voltage -= BATTERY_MIN_VOLTAGE;
    voltage /= (BATTERY_MAX_VOLTAGE-BATTERY_MIN_VOLTAGE);
    /* clip voltage to "acceptable range" */
    voltage  = voltage < 0.0k ? 0.0k : voltage;
    voltage  = voltage > 0.99k? 0.99k : voltage;
    /* convert to scale of 0->20 */
    voltage *= 20;
    return (int)voltage;
}
#endif
