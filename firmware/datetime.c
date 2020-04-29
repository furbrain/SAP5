#include <time.h>
#include <stdlib.h>
#include "datetime.h"
#include "display.h"
#include "mcc_generated_files/rtcc.h"
#include "ui.h"

void set_num_days(struct UI_MULTI_SELECT *sel);

struct UI_MULTI_SELECT date_select = {
    .offset = 0,
    .text = "2000-01-02",
    .numbers = {
       //max   min   curr  len pos cb
        {99,   20,   20,   2,  2,  NULL }, //years
        {12,   1,    5,    2,  5,  set_num_days}, //months
        {31,   1,    13,   2,  8,  NULL}
    }
};

struct UI_MULTI_SELECT time_select = {
    .offset = 30,
    .text = "15:43",
    .numbers = {
       //max   min   curr  len pos cb
        {23,   0,    15,   2,  0,  NULL }, //hours
        {59,   0,    43,    2,  3,  NULL}, //minutes
    }

};


int get_num_days(int year, int month) {
    switch(month) {
        case 3:
        case 5:
        case 8:
        case 10:
            return 30;
        case 2:
            if (year % 4 == 0) {
                return 29;
            } else {
                return 28;
            }
        default:
            return 31;
    }
}

void set_num_days(struct UI_MULTI_SELECT *sel) {
    int days = get_num_days(sel->numbers[0].current, sel->numbers[1].current);
    sel->numbers[2].max = days;
}

void datetime_set_date(int32_t a) {
    struct tm dt;    
    RTCC_TimeGet(&dt);    
    strftime(date_select.text, 12, "%Y-%m-%d", &dt);
    date_select.numbers[0].current = dt.tm_year - 100;
    date_select.numbers[1].current = dt.tm_mon + 1;
    date_select.numbers[2].current = dt.tm_mday;
    ui_multi_select(&date_select);
    RTCC_TimeGet(&dt);  // do this again so we don't lose time in minutes and seconds
    dt.tm_year = date_select.numbers[0].current + 100;
    dt.tm_mon = date_select.numbers[1].current - 1;
    dt.tm_mday = date_select.numbers[2].current;
    RTCC_TimeSet(&dt);
}

void datetime_set_time(int32_t a) {
    struct tm dt;    
    RTCC_TimeGet(&dt);    
    strftime(time_select.text, 20, "%H:%M", &dt);
    time_select.numbers[0].current = dt.tm_hour;
    time_select.numbers[1].current = dt.tm_min;
    ui_multi_select(&time_select);
    dt.tm_hour = time_select.numbers[0].current;
    dt.tm_min = time_select.numbers[1].current;            
    RTCC_TimeSet(&dt); 
}
