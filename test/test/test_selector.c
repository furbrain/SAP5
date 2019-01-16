#include "unity.h"
#include "selector.h"
#include "mock_display.h"
#include "mock_interface.h"
#include "mock_utils.h"
#include "font.h"

struct SELECTOR_CHOICES choices = {
    "0123456789*",
    11,
    0
};

void setUp(void)
{
    choices.end = 11;
    choices.current = 0;
    delay_ms_safe_Ignore();
    
}

void tearDown(void)
{
}

void expectDrawMarkers(int column) {
    render_data_to_page_Expect(1, column, up_marker, 16);
    render_data_to_page_Expect(6, column, down_marker, 16);
}

void expectEraseMarkers(int column) {
    render_data_to_page_Expect(1, column, empty_marker, 16);
    render_data_to_page_Expect(6, column, empty_marker, 16);
}

void test_selector_choose_and_immediate_return(void)
{
    char result;
    expectDrawMarkers(0);
    get_input_ExpectAndReturn(SINGLE_CLICK);
    expectEraseMarkers(0);

    result = selector_choose(&choices, 0);
    TEST_ASSERT_EQUAL(result, '0');
}

void test_selector_choose_and_select_next(void)
{
    char result;
    choices.current = 10;
    expectDrawMarkers(0);
    get_input_ExpectAndReturn(FLIP_DOWN);
    display_write_text_Expect(2, 0, "0", &large_font, false, true);
    get_input_ExpectAndReturn(SINGLE_CLICK);
    expectEraseMarkers(0);

    result = selector_choose(&choices, 0);
    TEST_ASSERT_EQUAL(result, '0');
}


void test_selector_choose_and_select_prev(void)
{
    char result;
    expectDrawMarkers(0);
    get_input_ExpectAndReturn(FLIP_UP);
    display_write_text_Expect(2, 0, "*", &large_font, false, true);
    get_input_ExpectAndReturn(SINGLE_CLICK);
    expectEraseMarkers(0);

    result = selector_choose(&choices, 0);
    TEST_ASSERT_EQUAL(result, '*');
}
