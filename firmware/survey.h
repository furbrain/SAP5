#ifndef _SURVEY_H
#define _SURVEY_H

#include <time.h>
#include <stdbool.h>

#define EIGHT_HOURS (8 * 60 * 60)

struct SURVEY {
    int number;
    int max_station;
    bool forward;
    time_t start_time;
};
#endif // _SURVEY_H

extern struct SURVEY survey_current;

/* find current survey - either most recent one if one started in last 8 hours,
   or start a new one */
void survey_current_init();

/* populate a survey structure with data from storage */
void survey_populate(struct SURVEY *survey, int number);

/* start a new survey */
void survey_start_new();
