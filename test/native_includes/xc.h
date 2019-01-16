#ifndef XC_H
#define XC_H
#include <stdint.h>

struct {
    uint8_t TRISB6;
} TRISBbits;
    
struct {
    uint8_t LATB6;
} LATBbits;

#define PORTBbits LATBbits
#define RB6 LATB6
#endif
