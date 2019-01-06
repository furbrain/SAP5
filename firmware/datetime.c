#include <time.h>
#include "datetime.h"
#include "display.h"
#include "selector.h"
#include "mcc_generated_files/rtcc.h"

int text2num(char text) {
    return text-'0';
}


int date_positions[] = {0,13,26,41,56,63,77,92,99,113};

int num_day_in_month(struct tm *dt) {
    //first digit was 0 or 1 - allow any number
    if (dt->tm_mday < 20) return 10;
    
    //deal with February
    if (dt->tm_mon==1) {
        return (dt->tm_year % 4) ? 9 : 10;        
    }
    
    //first digit was a 2 and not February - allow any number
    if (dt->tm_mday < 30) return 10;
    
    //one of the months with thirty days - allow 0 only
    if (dt->tm_mon==3) return 1;
    if (dt->tm_mon==5) return 1;
    if (dt->tm_mon==8) return 1;
    if (dt->tm_mon==10) return 1;
    //one of the months with 31 days - allow 0 or 1
    return 2;
}

void datetime_set_date(int32_t a) {
    struct SELECTOR_CHOICES choices = {"0123456789", 0, 10, 0};
    char text[20] = "";
    char chosen;
    struct tm dt;
    int i;
    RTCC_TimeGet(&dt);
    strftime(text, 20, "%Y-%m-%d", &dt);
    display_clear_screen(true);
    for (i=0; i< 10; ++i) {
        selector_write_char(text[i],date_positions[i]);
    }

    
    choices.end = 10;
    choices.current = text2num(text[2]);
    chosen = selector_choose(&choices,date_positions[2]);
    dt.tm_year = 100 + text2num(chosen)*10;
    
    choices.current = text2num(text[3]);
    chosen = selector_choose(&choices,date_positions[3]);
    dt.tm_year += text2num(chosen);
    
    choices.end = 2;
    choices.current = text2num(text[5]);
    chosen = selector_choose(&choices,date_positions[5]);
    dt.tm_mon = text2num(chosen)*10;

    choices.end = (chosen=='1') ? 3 : 10;
    choices.start = (chosen=='0') ? 1 : 0;
    choices.current = text2num(text[6]);
    chosen = selector_choose(&choices,date_positions[6]);
    dt.tm_mon += text2num(chosen);
    dt.tm_mon -= 1; //January is month 0...
    
    choices.start = 0;
    choices.end = (dt.tm_mon==1) ? 3 : 4;
    choices.current = text2num(text[8]);
    chosen = selector_choose(&choices,date_positions[8]);
    dt.tm_mday = text2num(chosen)*10;
    
    choices.start = (chosen=='0') ? 1 : 0;
    choices.end = num_day_in_month(&dt);
    choices.current = text2num(text[9]);
    chosen = selector_choose(&choices,date_positions[9]);
    dt.tm_mday += text2num(chosen);
    
    RTCC_TimeSet(&dt);
}

void datetime_set_time(int32_t a) {
    struct SELECTOR_CHOICES choices = {"0123456789", 0, 10, 0};
    char text[20] = "";
    char chosen;
    struct tm dt;
    int i;
    RTCC_TimeGet(&dt);
    strftime(text, 20, "%H:%M", &dt);
    display_clear_screen(true);
    for (i=0; i<5; ++i) {
        selector_write_char(text[i], 26+i*14);
    }

    choices.end = 3;
    choices.current = text2num(text[0]);
    chosen = selector_choose(&choices,26);
    dt.tm_hour  = text2num(chosen)*10;

    choices.end = (chosen=='2') ? 4: 10;
    choices.current = text2num(text[1]);
    chosen = selector_choose(&choices,26+14);
    dt.tm_hour  += text2num(chosen);

    choices.end = 6;
    choices.current = text2num(text[3]);
    chosen = selector_choose(&choices,26+14*3);
    dt.tm_min = text2num(chosen)*10;
    
    choices.end = 10;
    choices.current = text2num(text[4]);
    chosen = selector_choose(&choices,26+14*3);
    dt.tm_min += text2num(chosen);
    
    RTCC_TimeSet(&dt);
}
