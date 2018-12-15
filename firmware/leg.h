#ifndef _LEG_H
#define _LEG_H
#include <stdint.h>
#include <time.h>
#include <gsl/gsl_vector.h>
#include "mem_locations.h"
#include "survey.h"


#define MAX_LEG_COUNT (APP_LEG_SIZE / sizeof(struct LEG))
#define LEG_SPLAY 0xFF


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
struct LEG leg_create(time_t tm, uint16_t survey, uint8_t from, uint8_t to, gsl_vector *delta);

/* save a leg */
void leg_save(struct LEG *leg);

/* if leg spans a page boundary, then return the pointer to the start of the page *
 * otherwise return null */
void *leg_spans_boundary(const struct LEG *leg);


/* find a leg */
const struct LEG *leg_find(int survey, int index);

/* find the maximum station within a survey and also the time of the first leg */
void leg_get_survey_details(int survey, int *max_station, time_t *first_leg);

/*find most recent_leg*/
const struct LEG *leg_find_last(void);

/* convert a pair of stations to text, do not alter the returned string - owned by this module */
const char *leg_stations_to_text(uint8_t from, uint8_t to);

/* encode a pair of station numbers as an int32_t */
int32_t leg_stations_encode(uint8_t from, uint8_t to);

/* reverse encoding as done by above function */
void leg_stations_decode(int32_t, uint8_t *from, uint8_t *to);


#endif // _LEG_H
