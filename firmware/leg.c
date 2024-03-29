#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "leg.h"
#include "mem_locations.h"
#include "memory.h"
#include "utils.h"
#include "exception.h"
#include "sensors.h"
#include "survey.h"

union LEG_STORE leg_store PLACE_DATA_AT(APP_LEG_LOCATION) = {.raw = {[0 ... APP_LEG_SIZE-1]=0xff}};

char leg_stations_description[20];

void leg_create_and_store(int32_t code) {
    struct LEG leg;
    int i;
    int the_time = utils_get_time();
    gsl_vector* reading = sensors_get_last_reading();
    if (the_time<0) {
        THROW_WITH_REASON("Bad code",ERROR_UNSPECIFIED);
    }
    leg.tm = the_time;
    leg.survey = survey_current.number;
    leg_stations_decode(code, &leg.from, &leg.to);
    for(i=0; i<3; i++) {
        leg.delta[i] = gsl_vector_get(reading, i);
    }
    leg_save(&leg);
    survey_add_leg(&survey_current, &leg);
}

/* if leg spans a page boundary, then return the pointer to the start of the page *
 * otherwise return null */

void *leg_spans_boundary(const struct LEG *leg) {
    size_t addr = (size_t)(leg);
    size_t overlap = addr % 0x800;
    if (addr == (size_t)&leg_store)
        return (void*)leg;
    if (overlap < (0x800 - sizeof(struct LEG))) {
        return NULL;
    }
    addr = addr + 0x800 - overlap;
    if (addr >= (size_t)&leg_store.legs[MAX_LEG_COUNT]) {
        return NULL;
    }
    return (void*)addr;
}

static inline bool _is_valid(const struct LEG *leg) {
    return (leg->tm != -1);
}

void leg_save(struct LEG *leg) {
    CEXCEPTION_T e;
    const struct LEG *ptr = leg_store.legs;
    const struct LEG *leg_overflow = &leg_store.legs[MAX_LEG_COUNT];
    void *boundary;
    while ((ptr < leg_overflow) && _is_valid(ptr)) {
        ptr ++;
    }
    if (ptr >= leg_overflow) {
        ptr = leg_store.legs;
    }
    boundary = leg_spans_boundary(ptr);
    if (boundary) {
        memory_erase_page(boundary);
    }
    Try {
        memory_write_data(ptr, leg, sizeof(struct LEG));
    }
    Catch (e) {
        if (e==ERROR_FLASH_STORE_FAILED) 
            THROW_WITH_REASON("Save Leg failed",e);
    }
}


/* find a leg */
const struct LEG *leg_find(int survey, size_t index) {
    size_t i;
    size_t first_point=0xffffffff;
    time_t first_tm=LONG_MAX;
    size_t count = 0;
    const struct LEG *leg;
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

/*find most recent_leg*/
const struct LEG *leg_find_last(void) {
    time_t last_tm = LONG_MIN;
    uint16_t last_survey = 0;
    const struct LEG *leg  = NULL;
    const struct LEG *last_leg = NULL;
    // find last survey
    while ((leg=leg_enumerate(leg)) != NULL) {
        if (leg->survey > last_survey) {
            last_survey = leg->survey;
        }
    }
    //now find latest_time in last survey
    leg = NULL;
    while ((leg=leg_enumerate(leg)) != NULL) {
        if (leg->survey == last_survey) {
            if (leg->tm > last_tm) {
                last_leg = leg;
                last_tm = leg->tm;
            }
        }
    }
    return last_leg;
}

/* convert a pair of stations to text, do not alter the returned string - owned by this module */
const char *leg_stations_to_text(uint8_t from, uint8_t to) {
    char from_text[8];
    char to_text[8];
    if (from==LEG_SPLAY) {
        sprintf(from_text,"-");
    } else {
        sprintf(from_text, "%d", from);
    }
    if (to==LEG_SPLAY) {
        sprintf(to_text,"-");
    } else {
        sprintf(to_text, "%d", to);
    }
    sprintf(leg_stations_description, "%s  ->  %s", from_text, to_text);
    return leg_stations_description;
}


union ENCODER {
    int32_t code;
    struct {
    uint8_t from;
    uint8_t to;
    };
};

/* encode a pair of station numbers as an int32_t */
int32_t leg_stations_encode(uint8_t from, uint8_t to) {
    union ENCODER encoder;
    encoder.from = from;
    encoder.to = to;
    return encoder.code;
}

/* reverse encoding as done by above function */
void leg_stations_decode(int32_t code, uint8_t *from, uint8_t *to) {
    union ENCODER encoder;
    encoder.code = code;
    *from = encoder.from;
    *to = encoder.to;
}

/* return true if one station in leg is a splay */
bool leg_is_splay(const struct LEG *leg) {
    if (leg->from==LEG_SPLAY) return true;
    if (leg->to==LEG_SPLAY) return true;
    return false;
    
}

/* return the first valid leg in storage */
const struct LEG *leg_first(void) {
    const struct LEG *leg = &leg_store.legs[0];
    do {
        if (_is_valid(leg)) return leg;
        leg++;
    } while (leg < &leg_store.legs[MAX_LEG_COUNT]);
    //not found any valid legs
    return NULL;
}

/* get the next valid leg in storage , or return NULL if no more valid legs */
const struct LEG *leg_enumerate(const struct LEG *leg) {
    if (leg==NULL) return leg_first();
    while (++leg < &leg_store.legs[MAX_LEG_COUNT]) {
        if (_is_valid(leg)) return leg;
    }
    return NULL;
}