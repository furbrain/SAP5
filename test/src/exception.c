#include <xc.h>
#include <gsl/gsl_errno.h>
#include "exception.h"

const char exception_strings[][20] = {
    "Unspecified Error",
    "Menu Full",
    "Menu Empty",
    "String too big",
    "Mem store failed",
    "Survey not found",
    "Laser read failed",
    "Laser timed out",
    "No survey data",
    "Survey too big",
    "Survey is disjoint",
    "Accelerometer Error",
    "Magnetometer Error",
    "No bluetooth device",
    "BT no response",
    "BT overflow",
    "BT command failed",
    "Sim comms fail",
    "Hardware unknown",
    "Procedure aborted"
    };


const char global_exception_strings[][20] = {
    "Interrupt",
    "Invalid exception",
    "Invalid exception",
    "Invalid exception",
    "Address load error",
    "Address store error",
    "Bus fetch error",
    "Bus load/store err",
    "Syscall",
    "Breakpoint",
    "Reserved Instn",
    "CoCPU unusable",
    "Math Overflow",
    "Divide by Zero",
    "Invalid exception",
    "Invalid exception",
    "Implementn specific",
    "CorExtend Unusable",
    "CoCPU 2 unusable"
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


//omit this function if not using XC32...
#if 0
static unsigned int _excep_code;
static unsigned int _excep_addr;
static char _excep_reason[12];
void _general_exception_handler(void) {
    _excep_code=(_CP0_GET_CAUSE() & 0x0000007C) >> 2;
    _excep_addr=_CP0_GET_EPC();
    snprintf(_excep_reason,12,"0x%X",_excep_addr);
    exception_error_handler(_excep_reason, "exception", 0, _excep_code+2000);
}


void _simple_tlb_refill_exception_handler(void) {
    _general_exception_handler();
}

void _cache_err_exception_handler(void) {
    _general_exception_handler();
}

void _nmi_handler(void) {
    _general_exception_handler();
}
#endif



const char* exception_get_string(CEXCEPTION_T e) {
    if (e<500) 
        return exception_strings[e];
    else if (e<1500)
        return gsl_strerror((signed int)e-1000);
    else
        return global_exception_strings[e-2000];
}    

void exception_get_details(const char **reason, const char **file, int *line) {
    int last_divider = 0;
    int next_but_last_divider = 0;
    const char *ptr;
    ptr = exc_file;
    while (*ptr) {
        switch (*ptr) {
            case '\\':
            case '/':
                next_but_last_divider = last_divider;
                last_divider = (ptr-exc_file)+1;
                break;
        }
        ptr++;
    }
    *reason = exc_reason;
    *file = exc_file+next_but_last_divider;
    *line = exc_line;
}
