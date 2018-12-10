#ifndef _LEG_H
#define _LEG_H
#include <stdint.h>
#include <time.h>
#include "mem_locations.h"


#define MAX_LEG_COUNT (APP_LEG_SIZE / sizeof(struct LEG))

struct __attribute__((aligned(8))) LEG {
    time_t tm; // time of reading
    uint16_t survey; //survey number
    uint8_t from; //origin station
    uint8_t to; //destination station
    /* differential readings in m */
    double delta[3]; 
};


/* Note it is important to keep this as a union to ensure that no other data/ program 
 * code is kepts in the gap between the last leg and the top of its page in memory */
union LEG_STORE {
    CONST_STORE uint8_t raw[APP_LEG_SIZE];
    CONST_STORE struct LEG legs[MAX_LEG_COUNT];
};

extern union LEG_STORE leg_store;


/* create a leg */
struct LEG leg_create(time_t tm, uint16_t survey, uint8_t from, uint8_t to, double delta[3]);

/* save a leg */
int leg_save(struct LEG *leg);

/* if leg spans a page boundary, then return the pointer to the start of the page *
 * otherwise return null */
void *leg_spans_boundary(struct LEG *leg);


/* find a leg */
struct LEG *leg_find(uint16_t survey, int index);

/*find most recent_leg*/
struct LEG *leg_find_last(void);


#endif // _LEG_H
