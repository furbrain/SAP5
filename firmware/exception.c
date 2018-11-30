#include "exception.h"

const char exception_strings[][20] = {
    "Unspecified Error",
    "Menu Full",
    "Menu Empty",
    "String too big"
    };
    
const char* get_exception_string(CEXCEPTION_T e) {
    return exception_strings[e];
}    
