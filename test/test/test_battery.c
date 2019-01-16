#include "unity.h"
#include "CException.h"
#include "battery.h"
#include "mock_adc1.h"
#include "mock_utils.h"
#include <stdio.h>
#include <string.h>
#include <xc.h>

void test_battery_get_status(void) {
    TRISBbits.TRISB6 = 0;
    LATBbits.LATB6 = 1;
    TEST_ASSERT_EQUAL_INT(battery_get_status(), CHARGING);
    LATBbits.LATB6 = 0;
    TEST_ASSERT_EQUAL_INT(battery_get_status(), DISCHARGING);
}

int get_adc_result(double voltage) {
    return (int)(voltage*4096/3.0);
}


void setup_adc(double voltage) {
    ADC1_Initialize_Expect();
    ADC1_ChannelSelect_Expect(ADC1_BAT_SENSE);
    ADC1_Start_Expect();
    delay_ms_safe_Expect(1);
    ADC1_Stop_Expect();
    ADC1_IsConversionComplete_IgnoreAndReturn(true);
    ADC1_ConversionResultGet_ExpectAndReturn(get_adc_result(voltage));
    ADC1_ChannelSelect_Expect(ADC1_CHANNEL_VBG);
    ADC1_Start_Expect();
    delay_ms_safe_Expect(1);
    ADC1_Stop_Expect();
    ADC1_ConversionResultGet_ExpectAndReturn(get_adc_result(1.2));
}


void test_battery_get_voltage(void) {
    accum voltages[] = {2.4, 3.2, 3.7, 4.2, 4.5}; 
    int i;
    for (i=0; i<5; i++) {
        setup_adc(voltages[i]);
        TEST_ASSERT_DOUBLE_WITHIN(0.01, voltages[i], battery_get_voltage());
    }
}

void test_battery_get_units(void) {
    struct test_field {
        accum voltage;
        int units;
    };
    struct test_field test_cases[6] = {
        {2.0, 0},
        {BATTERY_MIN_VOLTAGE, 0},
        {(BATTERY_MIN_VOLTAGE+BATTERY_MAX_VOLTAGE)/2, 10},
        {BATTERY_MAX_VOLTAGE-0.01, 19},
        {BATTERY_MAX_VOLTAGE, 19},
        {5.0, 19}};
    int i;
    for (i=0; i<6; i++) {
        setup_adc(test_cases[i].voltage);
        TEST_ASSERT_EQUAL_INT(test_cases[i].units, battery_get_units());
    }    
}
