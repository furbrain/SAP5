#include "unity.h"
#include "exception.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

void suiteSetUp(void) {
    exception_init();
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_exception_get_string(void)
{
    TEST_ASSERT_EQUAL_STRING("Unspecified Error", exception_get_string(ERROR_UNSPECIFIED));
    TEST_ASSERT_EQUAL_STRING("Menu Full", exception_get_string(ERROR_MENU_FULL));
    TEST_ASSERT_EQUAL_STRING("output range error", exception_get_string(GSL_ERANGE+1000));
}

void test_gsl_error(void) 
{
    const char *reason;
    const char *file;
    int line;
    TEST_ASSERT_THROWS_GSL(gsl_fft_complex_radix2_forward(NULL,1,3), GSL_EINVAL);
    exception_get_details(&reason, &file, &line);
    printf("%s\n%s:%d",reason, file, line);
}

void test_get_details(void) {
    CEXCEPTION_T e;
    const char *reason;
    const char *file;
    int line;
    Try {
        exception_error_handler("Some Reason", "file.c", 120, ERROR_UNSPECIFIED);
    }
    Catch (e) {
        TEST_ASSERT_EQUAL(e, ERROR_UNSPECIFIED);
    }
    exception_get_details(&reason, &file, &line);
    TEST_ASSERT_EQUAL_STRING("Some Reason",reason);
    TEST_ASSERT_EQUAL_STRING("file.c", file);
    TEST_ASSERT_EQUAL(120, line);
}

void test_throw_with_reason(void) {
    CEXCEPTION_T e;
    const char *reason;
    const char *file;
    int line;
    Try {
        THROW_WITH_REASON("Some Reason2", ERROR_UNSPECIFIED);        
    }
    Catch (e) {
        TEST_ASSERT_EQUAL(e, ERROR_UNSPECIFIED);
        exception_get_details(&reason, &file, &line);
        TEST_ASSERT_EQUAL("Some Reason2",reason);        
        return;
    }
    TEST_FAIL_MESSAGE("Error not thrown");
 }
