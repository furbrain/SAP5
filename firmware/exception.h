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


const char* get_exception_string(CEXCEPTION_T e);
#endif // _EXCEPTION_H
