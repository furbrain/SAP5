#ifndef _EXCEPTION_H
#define _EXCEPTION_H

enum EXCEPTION_CODES {
    ERROR_UNSPECIFIED=0,
    ERROR_MENU_FULL,
    ERROR_MENU_EMPTY,
    ERROR_STRING_TOO_BIG
};


#define CEXCEPTION_T enum EXCEPTION_CODES

#include "CException.h"
    
#define THROW_WITH_REASON(reason, code) exception_error_handler(reason, __FILE__, __LINE__, code)

#define TEST_ASSERT_THROWS(_code_under_test, error_code)               \
{                                                                      \
  CEXCEPTION_T e;                                                      \
  Try {                                                                \
    _code_under_test;                                                  \
    TEST_FAIL_MESSAGE("Code under test did not assert " #error_code);  \
  } Catch(e) {                                                         \
    if (e!=error_code)                                                 \
        Throw(e);                                                      \
  }                                                                    \
}

#define TEST_ASSERT_THROWS_GSL(_code_under_test, error_code) TEST_ASSERT_THROWS(_code_under_test, error_code+1000)

void exception_init(void);
    
void exception_gsl_error_handler(const char *reason, const char *file, int line, int error_code);

void exception_error_handler(const char *reason, const char *file, int line, int error_code);

const char* exception_get_string(CEXCEPTION_T e);

void exception_get_details(const char **reason, const char **file, int *line);
#endif // _EXCEPTION_H
