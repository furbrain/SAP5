#include "unity.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_interface.h"
#include "mock_utils.h"
#include "mock_input.h"
#include "font.h"


void setUp(void)
{
}

void tearDown(void)
{
}

void expectDrawMarkers(int column) {
    render_data_to_screen_Expect(1, column, up_marker, 15);
    render_data_to_screen_Expect(6, column, down_marker, 15);
}

void expectEraseMarkers(int column) {
    render_data_to_screen_Expect(1, column, empty_marker, 15);
    render_data_to_screen_Expect(6, column, empty_marker, 15);
}

void expectEraseCharacter(int column) {
    render_data_to_screen_Expect(2, column, empty_marker, 14);
    render_data_to_screen_Expect(3, column, empty_marker, 14);
    render_data_to_screen_Expect(4, column, empty_marker, 14);
    render_data_to_screen_Expect(5, column, empty_marker, 14);
}

void test_get_digit() {
    struct test_field {
        int num;
        int pos;
        int result;
    };
    struct test_field test_cases[] = {
        {1, 0, 1},
        {0, 0, 0},
        {9, 0, 9},
        {15, 0, 5},
        {10, 1, 1},
        {19, 1, 1},
        {35, 1, 3},
        {100, 2, 1},
        {99, 2, 0},
        {199, 2, 1},
        {200, 2, 2},
        {235, 1, 3},
        {999, 3, 0},
    };
    int i;
    char message[20];
    for (i=0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        sprintf(message, "Iter: %d", i);
        TEST_ASSERT_EQUAL_MESSAGE(test_cases[i].result, get_digit(test_cases[i].num, test_cases[i].pos), message);
    }
}

void test_compare_nums() {
    struct test_field {
        int a;
        int b;
        int pos;
        bool result;
    };
    struct test_field test_cases[] = {
        {0, 0, 0, true},
        {0, 1, 0, false},
        {1, 0, 0, false},
        {12, 12, 0, true},
        {13, 14, 0, false},
        {12, 12, 1, true},
        {13, 14, 1, true},
        {19, 20, 1, false},
        {123, 100, 2, true},
        {123, 100, 1, false},
        {123, 124, 1, true}
    };
    struct test_field *ex;
    int i;
    char message[20];
    for (i=0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        sprintf(message, "Iter: %d", i);
        ex = &test_cases[i];
        TEST_ASSERT_EQUAL_MESSAGE(ex->result, compare_nums(ex->a, ex->b, ex->pos), message);
    }
}

void test_set_digit() {
    struct test_field {
        int num;
        int pos;
        int digit;
        int result;
    };
    struct test_field test_cases[] = {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 1, 1, 10},
        {0, 3, 2, 2000},
        {1978, 1, 8, 1988},
        {1978, 0, 3, 1973},
        {1978, 3, 2, 2978},
    };
    struct test_field *ex;
    int i;
    char message[20];
    for (i=0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        sprintf(message, "Iter: %d", i);
        ex = &test_cases[i];
        TEST_ASSERT_EQUAL_MESSAGE(ex->result, set_digit(ex->num, ex->pos, ex->digit), message);
    }
}