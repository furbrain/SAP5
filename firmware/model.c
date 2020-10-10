#include "model.h"
#include "leg.h"
#include "utils.h"
#include "exception.h"

//warning this code was written by drunk phil -but he's a fuckwit, so
//please could sober phil check it...
//need to consider what to do with splay legs...

//need to unionify this data storage with stuff in calibrate...
TESTABLE_STATIC struct MODEL_LEG model_legs[MODEL_MAX_STORAGE];
TESTABLE_STATIC int model_leg_count;

TESTABLE_STATIC struct MODEL_STATION model_stations[MODEL_MAX_STORAGE];
TESTABLE_STATIC int station_count;

TESTABLE_STATIC const struct LEG *processed_legs[MODEL_MAX_STORAGE];
TESTABLE_STATIC int processed_leg_count;

/*reset all lists*/
TESTABLE_STATIC
void reset_lists(void) {
    model_leg_count = 0;
    station_count = 0;
    processed_leg_count = 0;
}

/* add a station to the found list, along with its position
 * raise error if too many stations added */
TESTABLE_STATIC
struct MODEL_STATION *add_station(uint8_t number, double *pos) {
    struct MODEL_STATION *station;
    if (station_count >= MODEL_MAX_STORAGE) {
        THROW_WITH_REASON("Too many stations in survey to store", ERROR_SURVEY_TOO_BIG);
    }
    station = &model_stations[station_count];
    station->number = number;
    station->pos[0] = pos[0];
    station->pos[1] = pos[1];
    station->pos[2] = pos[2];
    ++station_count;
    return station;
    //FIXME raise error if too many stations...
}

/*set first from station in survey to (0,0,0)
  Throws ERROR_SURVEY_PROCESS_FAIL if no legs found*/
TESTABLE_STATIC
void initialise_first_station(uint16_t survey){
    size_t i;
    for (i=0; i < MAX_LEG_COUNT; i++) {
        if (leg_store.legs[i].survey == survey) {
            add_station(leg_store.legs[i].from, (double[3]){0,0,0});
            return;
        }
    }
    THROW_WITH_REASON("No survey legs found when initialising", ERROR_NO_SURVEY_DATA);
}

/*if a station has already been found, return a pointer to it
 * otherwise return null*/
TESTABLE_STATIC
struct MODEL_STATION *find_station(uint8_t number) {
    int i;
    struct MODEL_STATION *station;
    if (number==LEG_SPLAY) return NULL;
    for (i=0; i< station_count; i++) {
        station = &model_stations[i];
        if (station->number == number) {
            return station;
        }
    }
    return NULL;
}

/* add a model leg to the list*/
TESTABLE_STATIC
void add_leg(const struct MODEL_STATION *from, const struct MODEL_STATION *to) {
    struct MODEL_LEG *leg;
    //FIXME raise error if too many legs
    if (model_leg_count >= MODEL_MAX_STORAGE) {
        THROW_WITH_REASON("Too many legs in survey to store", ERROR_SURVEY_TOO_BIG);
    }
    leg = &model_legs[model_leg_count];
    leg->from = from;
    leg->to = to;
    ++model_leg_count;
}

/*test whether a leg has been processed*/
TESTABLE_STATIC
bool leg_has_been_processed(const struct LEG *leg) {
    int i;
    for (i=0; i<processed_leg_count; ++i) {
        if (processed_legs[i] == leg) return true;
    }
    return false;
}

/*mark a survey leg as having been processed*/
TESTABLE_STATIC
void mark_leg_as_processed(const struct LEG *leg) {
    //FIXME raise error if too many legs
    if (processed_leg_count >= MODEL_MAX_STORAGE) {
        THROW_WITH_REASON("Too many legs in survey to store", ERROR_SURVEY_TOO_BIG);
    }
    processed_legs[processed_leg_count] = leg;
    ++processed_leg_count;
    }

/*with arrays of double[3], add offset to origin and store the answer in result*/
TESTABLE_STATIC
void add_delta(const double *origin, const double *offset, double *result) {
    int i;
    for (i = 0; i<3; ++i) {
        result[i] = origin[i] + offset[i];
    }
}

/*with arrays of double[3], subtract offset from origin and store the answer in result*/
TESTABLE_STATIC
void sub_delta(const double *origin, const double *offset, double *result) {
    int i;
    for (i = 0; i<3; ++i) {
        result[i] = origin[i] - offset[i];
    }
}

TESTABLE_STATIC
struct MODEL_STATION *add_station_and_leg(const struct MODEL_STATION *known, const struct LEG *leg, bool forwards) {
    struct MODEL_STATION *unknown;
    double next_pos[3];
    if (forwards) {
        add_delta(known->pos, leg->delta, next_pos);
        unknown = add_station(leg->to, next_pos);
    } else {
        sub_delta(known->pos, leg->delta, next_pos);
        unknown = add_station(leg->from, next_pos);
    }
    add_leg(known, unknown);
    mark_leg_as_processed(leg);
    return unknown;
}

/* generate a model of the survey given by survey*/
void model_generate(uint16_t survey, struct MODEL_CAVE *cave) {
    const struct LEG *leg;
    struct MODEL_STATION *from;
    struct MODEL_STATION *to;
    struct MODEL_STATION *fake;
    size_t i;
    reset_lists();
    //set first station to 0,0,0
    initialise_first_station(survey);
    while (true) {
        bool changed = false;
        bool complete = true;
        for (i=0; i<MAX_LEG_COUNT; i++) {
            leg = &leg_store.legs[i];
            if (leg->survey == survey) {
                if (leg_has_been_processed(leg)) continue;
                from = find_station(leg->from);
                to = find_station(leg->to);
                if (from) {
                    fake = add_station_and_leg(from, leg, true);
                    if (to) {
                        add_leg(to, fake);
                    }
                    changed = true;
                } else if (to) {
                    add_station_and_leg(to, leg, false);
                    changed = true;
                } else {
                    complete = false;
                }
            }
        }
        if (complete) break; //hurrah we're done
        if (!changed) THROW_WITH_REASON("Survey is not connected", ERROR_SURVEY_IS_DISJOINT);
        //otherwise re-cycle
    }
    cave->legs = model_legs;
    cave->leg_count = model_leg_count;
    cave->stations = model_stations;
    cave->station_count = station_count;
}
