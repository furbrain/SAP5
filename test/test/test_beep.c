#include "unity.h"
#include "beep.h"
#include "mock_mccp2_compare.h"
#include "mock_utils.h"

void test_beep(void)
{
    struct test_field {
        double freq;
        int priVal;
    };
    struct test_field test_cases[] = {
        {776.719789729, 271},
        {353.128494484, 123},
        {617.565870557, 215},
        {400.212392031, 139},
        {882.711397609, 308},
        {640.332592833, 223},
        {676.122791673, 236},
        {272.266804054, 95},
        {725.692102224, 253},
        {605.095842355, 211}
    };
    // test middle c
    int i;
    for(i=0;i<10;i++) {
        MCCP2_COMPARE_SingleCompare16ValueSet_Expect(test_cases[i].priVal);
        MCCP2_COMPARE_Start_Expect();
        delay_ms_safe_Expect(1000);
        MCCP2_COMPARE_Stop_Expect();
        beep(test_cases[i].freq, 1000);
    }
}
