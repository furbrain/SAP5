#include "unity.h"
#include "exception.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_exception_NeedToImplement(void)
{
    TEST_ASSERT_EQUAL_STRING("Unspecified Error",get_exception_string(ERROR_UNSPECIFIED));
    TEST_ASSERT_EQUAL_STRING("Menu Full",get_exception_string(ERROR_MENU_FULL));
}
