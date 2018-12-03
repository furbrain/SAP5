#include <gsl/gsl_errno.h>
#include "exception.h"

const char exception_strings[][20] = {
    "Unspecified Error",
    "Menu Full",
    "Menu Empty",
    "String too big"
    };

static const char *exc_reason = "";
static const char *exc_file = "";
static int exc_line = 0;

void exception_init(void) {
    gsl_set_error_handler(exception_gsl_error_handler);
}

void exception_gsl_error_handler(const char *reason, const char *file, int line, int error_code) {
    exception_error_handler(reason, file, line, error_code+1000);
}
void exception_error_handler(const char *reason, const char *file, int line, int error_code) {
    exc_reason = reason;
    exc_file = file;
    exc_line = line;
    Throw(error_code);
}

const char* exception_get_string(CEXCEPTION_T e) {
    if (e>500) 
        return gsl_strerror(e-1000);
    else
        return exception_strings[e];
}    

void exception_get_details(const char **reason, const char **file, int *line) {
    *reason = exc_reason;
    *file = exc_file;
    *line = exc_line;
}
