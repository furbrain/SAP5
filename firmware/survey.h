#ifndef _SURVEY_H
#define _SURVEY_H

#include <time.h>
struct SURVEY {
    int number;
    int max_station;
    bool forward;
    time_t start_time;
}
#endif // _SURVEY_H

/* find current survey - either most recent one if one started in last 8 hours,
   or start a new one */
struct SURVEY survey_get_current();


/* get from and to stations for next leg for this survey */
void survey_get_next_leg(struct SURVEY survey, int *from, int *to);

/* record that we have added a leg to this survey */
void survey_add_leg(struct SURVEY *survey, struct LEG *leg);


