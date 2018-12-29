#include "utils.h"
#include "leg.h"
#include "survey.h"


struct SURVEY survey_current;

/* find current survey - either most recent one if one started in last 8 hours,
   or start a new one */
void survey_current_init() {
    const struct LEG *last_leg;
    time_t now;
    last_leg = leg_find_last();
    if (last_leg) {
        survey_current.number = last_leg->survey;
        now = utils_get_time();
        if (now > (last_leg->tm + EIGHT_HOURS)) {
            survey_start_new();
        } else {
            survey_populate(&survey_current, survey_current.number);
        }
    } else {
        survey_current.number = 0;
        survey_start_new();
    }
}

/* populate a survey structure with data from storage */
void survey_populate(struct SURVEY *survey, int number) {
    survey->number = number;
    leg_get_survey_details(number, &survey->max_station, &survey->start_time, &survey->last_leg_forward);
}


/* start a new survey */
void survey_start_new() {
    survey_current.number += 1;
    survey_current.max_station = 1;
    survey_current.last_leg_forward = false;
    survey_current.start_time = utils_get_time();
}

/* update a survey with details of a leg*/
void survey_add_leg(struct SURVEY *survey, struct LEG *leg){
    int max_station = (leg->from > leg->to) ? leg->from : leg->to;
    if (max_station > survey->max_station) {
        survey->max_station = max_station;
        survey->last_leg_forward = (leg->to > leg->from);
    }
}
