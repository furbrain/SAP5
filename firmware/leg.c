#include <limits.h>
#include <stdbool.h>

#include "leg.h"
#include "memory.h"
#include "utils.h"
#include "exception.h"

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

static inline bool _is_valid(struct LEG *leg) {
    return (leg->tm != ULONG_MAX);
}

void leg_save(struct LEG *leg) {
    CEXCEPTION_T e;
    struct LEG *ptr = leg_store.legs;
    struct LEG *leg_overflow = &leg_store.legs[MAX_LEG_COUNT];
    void *boundary;
    while ((ptr < leg_overflow) && _is_valid(ptr)) {
        ptr ++;
    }
    if (ptr >= leg_overflow) {
        ptr = leg_store.legs;
    }
    boundary = leg_spans_boundary(ptr);
    if (boundary) {
        erase_page(boundary);
    }
    Try {
        write_data(ptr, leg, sizeof(struct LEG));
    }
    Catch (e) {
        if (e==ERROR_FLASH_STORE_FAILED) 
            THROW_WITH_REASON("Save Leg failed",e);
    }
}


/* find a leg */
struct LEG *leg_find(int survey, int index) {
    int i;
    int first_point=0xffffffff;
    time_t first_tm=LONG_MAX;
    int count = 0;
    struct LEG *leg;
    /* first scan through to find the first relevant point */
    for (i=0; i< MAX_LEG_COUNT; i++) {
        leg = &leg_store.legs[i];
        if (_is_valid(leg) && (leg->survey == survey)) {
            if (leg->tm < first_tm) {
                first_point = i;
                first_tm = leg->tm;
            }
        }
    }
    if (first_point==0xffffffff) return NULL;
    for (i=first_point; i< (first_point + MAX_LEG_COUNT); i++) {
        leg = &leg_store.legs[i % MAX_LEG_COUNT];
        if (_is_valid(leg) && (leg->survey == survey)) {
            if (count==index) {
                return leg;
            } else {
                count++;
            }
        }
    }
    return NULL;
}

/* find the maximum station within a survey and also the time of the first leg */
void leg_get_survey_details(int survey, int *max_station, time_t *first_leg) {
    int max_st = INT_MIN;
    time_t first_tm = LONG_MAX;
    int i;
    struct LEG *leg;
    for (i=0; i< MAX_LEG_COUNT; i++) {
        leg = &leg_store.legs[i % MAX_LEG_COUNT];
        if (_is_valid(leg) && (leg->survey == survey)) {
            if (leg->tm < first_tm) {
                first_tm = leg->tm;
            }
            if (leg->from > max_st) {
                max_st = leg->from;
            }
            if (leg->to > max_st) {
                max_st = leg->to;
            }
        }
    }
    if (max_st == INT_MIN && first_tm==LONG_MAX) 
        THROW_WITH_REASON("No legs found for survey", ERROR_SURVEY_NOT_FOUND);
    *max_station = max_st;
    *first_leg = first_tm;
}

/*find most recent_leg*/
struct LEG *leg_find_last(void) {
    int i;
    time_t last_tm = LONG_MIN;
    int16_t last_survey = 0;
    struct LEG *leg;
    struct LEG *last_leg = NULL;
    // find last survey
    for (i=0; i< MAX_LEG_COUNT; i++) {
        leg = &leg_store.legs[i];
        if (_is_valid(leg)) {
            if (leg->survey > last_survey) {
                last_survey = leg->survey;
            }
        }
    }
    //now find latest_time in last survey
    for (i=0; i< MAX_LEG_COUNT; i++) {
        leg = &leg_store.legs[i];
        if (_is_valid(leg) && (leg->survey == last_survey)) {
            if (leg->tm > last_tm) {
                last_leg = leg;
                last_tm = leg->tm;
            }
        }
    }
    return last_leg;
}

