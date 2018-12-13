#include "utils.h"
#include "leg.h"
#include "survey.h"


struct SURVEY survey_current;

/* find current survey - either most recent one if one started in last 8 hours,
   or start a new one */
void survey_init() {
    struct LEG *last_leg;
    time_t now;
    last_leg = leg_find_last();
    if (last_leg) {
        survey_current.number = last_leg->survey;
        now = utils_get_time();
        if (now > (last_leg->tm + EIGHT_HOURS)) {
            survey_start_new();
        } else {
            leg_get_survey_details(survey_current.number, 
                                   &survey_current.max_station, 
                                   &survey_current.start_time);
        }
    } else {
        survey_current.number = 0;
        survey_start_new();
    }
}

/* start a new survey */
void survey_start_new() {
    survey_current.number += 1;
    survey_current.max_station = 1;
    survey_current.forward = true;
    survey_current.start_time = utils_get_time();
}


