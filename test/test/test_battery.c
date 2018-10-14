#include "unity.h"
#include "CException.h"
#include "battery.h"
#include "mock_adc1.h"
#include <stdio.h>
#include <string.h>
#include <xc.h>

void test_battery_charging(void) {
    TRISBbits.TRISB6 = 0;
    LATBbits.LATB6 = 1;
    TEST_ASSERT_EQUAL_INT(get_bat_status(), CHARGING);
}

void test_battery_discharging(void) {
    TRISBbits.TRISB6 = 0;
    LATBbits.LATB6 = 0;
    TEST_ASSERT_EQUAL_INT(get_bat_status(), DISCHARGING);
}

int get_adc_result(double voltage) {
    return (int)(voltage*4096/3.0);
}


void setup_adc(double voltage) {
    ADC1_Initialize_Expect();
    ADC1_ChannelSelect_Expect(ADC1_BAT_SENSE);
    ADC1_Start_Expect();
    ADC1_Stop_Expect();
    ADC1_IsConversionComplete_IgnoreAndReturn(true);
    ADC1_ConversionResultGet_ExpectAndReturn(get_adc_result(voltage));
    ADC1_ChannelSelect_Expect(ADC1_CHANNEL_VBG);
    ADC1_Start_Expect();
    ADC1_Stop_Expect();
    ADC1_ConversionResultGet_ExpectAndReturn(get_adc_result(1.2));
}


void test_battery_get_charge(void) {
    accum voltages[] = {2.4k, 3.2k, 3.7k, 4.2k, 4.5k}; 
    int i;
    for (i=0; i<5; i++) {
        setup_adc(voltages[i]);
        TEST_ASSERT_DOUBLE_WITHIN(0.01, voltages[i], get_bat_charge());
    }
}

void test_battery_units(void) {
    struct test_field {
        accum voltage;
        int units;
    };
    struct test_field test_cases[5] = {
        {2.4, 0},
        {3.2, 0},
        {3.7, 9},
        {4.2, 19},
        {4.5, 19}};
    int i;
    for (i=0; i<5; i++) {
        setup_adc(test_cases[i].voltage);
        TEST_ASSERT_EQUAL_INT(test_cases[i].units, battery_get_units());
    }    
}
