#ifndef _MODEL_H
#define _MODEL_H
#include <stdint.h>
#include <stdbool.h>

#define MODEL_MAX_STORAGE 256

struct MODEL_STATION {
    uint8_t number;
    double pos[3];
};

struct MODEL_LEG {
    const struct MODEL_STATION *from;
    const struct MODEL_STATION *to;
};

struct MODEL_CAVE {
    struct MODEL_LEG *legs;
    int leg_count;
    struct MODEL_STATION *stations;
    int station_count;
};

/* generate a model of the survey given by survey*/
void model_generate(uint16_t survey, struct MODEL_CAVE *cave);

#ifdef TEST
extern struct MODEL_LEG model_legs[];
extern int model_leg_count;

extern struct MODEL_STATION model_stations[];
extern int station_count;

extern const struct LEG *processed_legs[];
extern int processed_leg_count;


/*reset all lists*/
void reset_lists();

/*set first from station in survey to (0,0,0)
  Throws ERROR_SURVEY_PROCESS_FAIL if no legs found*/
void initialise_first_station(uint16_t survey);

/*if a station has already been found, return a pointer to it
 * otherwise return null*/
struct MODEL_STATION *find_station(uint8_t number);

/* add a station to the found list, along with its position*/
struct MODEL_STATION *add_station(uint8_t number, double *pos);

/* add a model leg to the list*/
void add_leg(const struct MODEL_STATION *from, const struct MODEL_STATION *to);

/*test whether a leg has been processed*/
bool leg_has_been_processed(const struct LEG *leg);

/*mark a survey leg as having been processed*/
void mark_leg_as_processed(const struct LEG *leg);

/*with arrays of double[3], add offset to origin and store the answer in result*/
void add_delta(const double *origin, const double *offset, double *result);

/*with arrays of double[3], subtract offset from origin and store the answer in result*/
void sub_delta(const double *origin, const double *offset, double *result);

/*create a unknown station from a leg and a given known station...*/
struct MODEL_STATION *add_station_and_leg(const struct MODEL_STATION *known, 
                                          const struct LEG *leg, 
                                          bool forwards);
#endif
#endif // _MODEL_H
