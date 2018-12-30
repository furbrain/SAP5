#include <xc.h>
#include "battery.h"
#include "mcc_generated_files/adc1.h"


enum BAT_STATUS battery_get_status(){
    
	if (PORTBbits.RB6) return CHARGING;
    return DISCHARGING;
}

#ifndef BOOTLOADER
double battery_get_voltage(){
	double bat_voltage, bg_voltage;
    int i;
    ADC1_Initialize();
    ADC1_ChannelSelect(ADC1_BAT_SENSE);
    ADC1_Start();
    //Provide Delay
    delay_ms_safe(1);
    ADC1_Stop();
    while(!ADC1_IsConversionComplete())
    {
        ADC1_Tasks();   
    }
    //bat_voltage = ADC1_ConversionResultGet();
    bat_voltage = ADC1BUF0;
    ADC1_ChannelSelect(ADC1_CHANNEL_VBG);
    ADC1_Start();
    //Provide Delay
    delay_ms_safe(1);
    ADC1_Stop();
    while(!ADC1_IsConversionComplete())
    {
        ADC1_Tasks();   
    }
    //bg_voltage = ADC1_ConversionResultGet();
    bg_voltage = ADC1BUF0;
    
	return (bat_voltage);
}

int battery_get_units() {
    double voltage;
    int result;
    voltage = battery_get_voltage();
    /* scale voltage to range of 0->1, where 0 is minimum acceptable and 1 is maximum acceptable */
    voltage -= BATTERY_MIN_VOLTAGE;
    voltage /= (BATTERY_MAX_VOLTAGE-BATTERY_MIN_VOLTAGE);
    /* clip voltage to "acceptable range" */
    voltage  = voltage < 0.0 ? 0.0 : voltage;
    voltage  = voltage > 0.99? 0.99 : voltage;
    /* convert to scale of 0->20 */
    voltage *= 20;
    return (int)voltage;
}
#endif
