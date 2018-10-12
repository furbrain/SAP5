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


void test_battery_fully_charged(void) {
    setup_adc(4.2);
    TEST_ASSERT_DOUBLE_WITHIN(0.01,4.2,get_bat_charge());
}

void test_battery_half_charged(void) {
    setup_adc(3.7);
    TEST_ASSERT_DOUBLE_WITHIN(0.01,3.7,get_bat_charge());
}

void test_battery_empty(void) {
    setup_adc(3.0);
    TEST_ASSERT_DOUBLE_WITHIN(0.01,3.0,get_bat_charge());
}

void test_battery_undercharged(void) {
    setup_adc(2.4);
    TEST_ASSERT_DOUBLE_WITHIN(0.01,2.4,get_bat_charge());
}

void test_battery_overcharged(void) {
    setup_adc(4.5);
    TEST_ASSERT_DOUBLE_WITHIN(0.01,4.5,get_bat_charge());
}
