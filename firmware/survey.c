#include "utils.h"
#include "leg.h"
#include "survey.h"


struct SURVEY survey_current = {0};

/* find current survey - either most recent one if one started in last 8 hours,
   or start a new one */
void survey_current_init() {
    const struct LEG *last_leg;
    time_t now;
    last_leg = leg_find_last();
    if (last_leg) {
        now = utils_get_time();
        if (now > (last_leg->tm + EIGHT_HOURS)) {
            survey_current.number = last_leg->survey;
            survey_start_new();
        } else {
            survey_populate(&survey_current, last_leg->survey);
        }
    } else {
        survey_current.number = 0;
        survey_start_new();
    }
}

/* populate a survey structure with data from storage */
void survey_populate(struct SURVEY *survey, int number) {
    const struct LEG *leg = NULL;
    survey->number = number;
    survey->max_station = 0;
    while((leg = leg_enumerate(leg)) != NULL) {
        if (leg->survey==number) {
            survey_add_leg(survey, leg);
        }
    }
}


/* start a new survey */
void survey_start_new() {
    survey_current.number += 1;
    survey_current.max_station = 1;
    survey_current.last_leg_forward = false;
    survey_current.start_time = utils_get_time();
}


/* update a survey with details of a leg*/
void survey_add_leg(struct SURVEY *survey, const struct LEG *leg){
    if (survey->start_time > leg->tm) {
        survey->start_time = leg->tm;
    }
    int max_station = (leg->from > leg->to) ? leg->from : leg->to;
    if (leg_is_splay(leg)) return; //splay leg, so do not change survey
    if (max_station > survey->max_station) {
        survey->max_station = max_station;
        survey->last_leg_forward = (leg->to > leg->from);
    }
}
