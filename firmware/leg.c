#include <limits.h>

#include "leg.h"
#include "storage.h"
#include "memory.h"
#include "utils.h"

union LEG_STORE leg_store PLACE_DATA_AT(APP_LEG_LOCATION) = {.raw = {[0 ... APP_LEG_SIZE-1]=0xff}};

struct LEG leg_create(time_t tm, uint16_t survey, uint8_t from, uint8_t to, double delta[3]) {
    struct LEG leg;
    int i;
    leg.tm = tm;
    leg.survey = survey;
    leg.from = from;
    leg.to = to;
    for(i=0; i<3; i++) leg.delta[i] = delta[i];
    return leg;
    
}

/* if leg spans a page boundary, then return the pointer to the start of the page *
 * otherwise return null */

void *leg_spans_boundary(struct LEG *leg) {
    size_t addr = (size_t)(leg+1);
    size_t overlap = addr % 0x800;
    if (overlap==0) return NULL;
    if (overlap <= sizeof(struct LEG)) {
        return (void *)(addr - overlap);
    } else {    
        return NULL;
    }
}


int leg_save(struct LEG *leg) {
    struct LEG *ptr = leg_store.legs;
    struct LEG *leg_overflow = &leg_store.legs[MAX_LEG_COUNT];
    void *boundary;
    int res;
    while ((ptr < leg_overflow) && (ptr->tm != ULONG_MAX)) {
        ptr ++;
    }
    if (ptr >= leg_overflow) {
        ptr = leg_store.legs;
    }
    boundary = leg_spans_boundary(ptr);
    if (boundary) {
        erase_page(boundary);
    }
    res =  write_data(ptr, leg, sizeof(struct LEG));
    return res;
    }

