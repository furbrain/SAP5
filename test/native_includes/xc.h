#ifndef XC_H
#define XC_H
#include <stdint.h>

struct {
    uint8_t TRISB6;
} TRISBbits;
    
struct {
	uint8_t LATB5;
    uint8_t LATB6;
} LATBbits;

struct {
	uint8_t BGREQ;
} AD1CON5bits;

#define PORTBbits LATBbits
#define RB6 LATB6
#define RB5 LATB5
#endif
