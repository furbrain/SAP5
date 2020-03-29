#include "config.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include "measure.h"
#include "interface.h"
#include "input.h"
#include "display.h"
#include "sensors.h"
#include "selector.h"
#include "maths.h"
#include "laser.h"
#include "menu.h"
#include "utils.h"
#include "survey.h"
#include "leg.h"
#include "gsl_static.h"
#include "exception.h"
#include "beep.h"
#include "images.h"

#define FEET_PER_METRE 3.281
#define DEGREES_PER_RADIAN 57.296
#define GRADS_PER_DEGREE 1.111111111
#define NUM_SENSOR_READINGS 10

#define METRIC_UNIT 'm'
#define IMPERIAL_UNIT '\''
#define DEGREE_SYMBOL '`'
#define GRAD_SYMBOL 'g'

GSL_VECTOR_DECLARE(measure_orientation, 3);

bool measure_requested = false;

DECLARE_EMPTY_MENU(measure_menu, 12);
DECLARE_EMPTY_MENU(storage_menu, 12);

TESTABLE_STATIC
void get_reading(gsl_vector *orientation){
	double distance;

	display_off();
	laser_on();
	delay_ms_safe(20);
    sensors_get_orientation(orientation, SAMPLES_PER_READING);
    distance = laser_read(LASER_MEDIUM, 3000);
    gsl_vector_scale(orientation, distance);
    display_on();
    laser_off();
}

/* calculate extension from current readings */
TESTABLE_STATIC
double get_extension(gsl_vector *orientation) {
    return gsl_hypot(gsl_vector_get(orientation, 0), gsl_vector_get(orientation, 1)); 
}


/* calculate total distance of this reading */
TESTABLE_STATIC
double get_distance(gsl_vector *orientation) {
    return gsl_blas_dnrm2(orientation);
}


/* calculate compass and inclination from an orientation */
void measure_calculate_bearings(gsl_vector *orientation, double *compass, double *inclination){
    *compass = atan2(gsl_vector_get(orientation, 0), gsl_vector_get(orientation, 1)) * DEGREES_PER_RADIAN;
	if (*compass<0) *compass+=360;
	*inclination = atan2(gsl_vector_get(orientation, 2), get_extension(orientation))*DEGREES_PER_RADIAN;
}

/* add a set of polar entries to a menu */
TESTABLE_STATIC
void add_polar_entries_to_menu(gsl_vector *orientation, struct menu *menu) {
    double compass, inclination;
    double degree_scale = (config.display_style==GRAD) ? GRADS_PER_DEGREE : 1.0;
    double length_scale = (config.length_units==IMPERIAL) ? FEET_PER_METRE : 1.0;
    char length_unit = (config.length_units==IMPERIAL) ? IMPERIAL_UNIT : METRIC_UNIT;
    char angle_symbol = (config.display_style==GRAD) ? GRAD_SYMBOL : DEGREE_SYMBOL;
    char text[30];
    measure_calculate_bearings(orientation, &compass, &inclination);

    sprintf(text, "%05.1f%c", compass * degree_scale, angle_symbol);
    menu_append_submenu(menu, text, &main_menu);

    sprintf(text, "%+.1f%c", inclination * degree_scale, angle_symbol);
    menu_append_submenu(menu, text, &main_menu);

    sprintf(text, "Dist  %.2f%c", get_distance(orientation) * length_scale, length_unit);
    menu_append_submenu(menu, text, &main_menu);

    sprintf(text, "Ext  %.2f%c", get_extension(orientation) * length_scale, length_unit);
    menu_append_submenu(menu, text, &main_menu);
}

/* add a set of cartesian entries to a menu */
TESTABLE_STATIC
void add_cartesian_entries_to_menu(gsl_vector *orientation, struct menu *menu) {
    const char *format[] = {"E: %+.2f%c", "N: %+.2f%c","V: %+.2f%c"};
    double length_scale = (config.length_units==IMPERIAL) ? FEET_PER_METRE : 1.0;
    char length_unit = (config.length_units==IMPERIAL) ? IMPERIAL_UNIT : METRIC_UNIT;
    char text[30];
    int i;
    for (i=0; i<3; i++) {
        sprintf(text, format[i], gsl_vector_get(orientation, i) * length_scale, length_unit);
        menu_append_submenu(menu, text, &main_menu);
    }
    sprintf(text, "Ext  %.2f%c", get_extension(orientation) * length_scale, length_unit);
    menu_append_submenu(menu, text, &main_menu);
}


static
void store_leg(int32_t code) {
    struct LEG leg;
    int the_time;
    the_time = utils_get_time();
    if (the_time<0) {
        THROW_WITH_REASON("Bad code",ERROR_UNSPECIFIED);
    }
    leg = leg_create(the_time, survey_current.number, 0, 0, &measure_orientation);
    leg_stations_decode(code, &leg.from, &leg.to);
    leg_save(&leg);
    survey_add_leg(&survey_current, &leg);
}

static
void add_storage_menu_entry(struct menu *menu, uint8_t from, uint8_t to) {
    const char *text;
    int32_t code;
    text = leg_stations_to_text(from, to);
    code = leg_stations_encode(from, to);
    menu_append_action(menu, text, store_leg, code);
}

int idx2pos(int i) {
    return 8 + i*14;
}

int txt2num(char x) {
    return x-'0';
}

void custom_storage(int32_t last) {
    struct SELECTOR_CHOICES choices = {"0123456789", 0, 10, 0};
    char text[20] = "";
    char chosen;
    int i;
    int max_tens = last / 10;
    int max_units = last % 10;
    int from  = last -1;
    int to = last;
    int digit;
    sprintf(text, "%02d -> %02d", from, to);
    display_clear_screen(true);
    for (i=0; i<8; ++i) {
        selector_write_char(text[i], idx2pos(i));
    }

    
    choices.end = max_tens + 1;
    choices.current = from / 10;
    chosen = selector_choose(&choices, idx2pos(0));
    digit = txt2num(chosen);
    if (digit == max_tens) {
        choices.end = max_units+1;
    } else {
        choices.end = 10;
    }
    choices.current = from % 10;
    from = digit * 10;
    chosen = selector_choose(&choices, idx2pos(1));
    from += txt2num(chosen);

    choices.end = max_tens + 1;
    choices.current = to / 10;
    chosen = selector_choose(&choices, idx2pos(6));
    digit = txt2num(chosen);
    if (digit == max_tens) {
        choices.end = max_units+1;
    } else {
        choices.end = 10;
    }
    choices.current = to % 10;
    to = digit * 10;
    chosen = selector_choose(&choices, idx2pos(7));
    to += txt2num(chosen);
    //FIXME need to do summat with these...
}

TESTABLE_STATIC
void setup_storage_menu(void) {
    int last = survey_current.max_station;
    menu_clear(&storage_menu);
    add_storage_menu_entry(&storage_menu, last, last+1);
    add_storage_menu_entry(&storage_menu, last, LEG_SPLAY);
    add_storage_menu_entry(&storage_menu, last+1, last);
    add_storage_menu_entry(&storage_menu, last+1, LEG_SPLAY);
    menu_append_action(&storage_menu, "Custom", custom_storage, last+2);
    menu_append_back(&storage_menu, "Back");
}

void measure_show_reading(gsl_vector *orientation) {
    // set up menus
    menu_clear(&measure_menu);
    if (config.display_style==CARTESIAN) {
        add_cartesian_entries_to_menu(orientation, &measure_menu);
    } else {
        add_polar_entries_to_menu(orientation, &measure_menu);
    }
    setup_storage_menu();
    menu_append_submenu(&measure_menu, "Store", &storage_menu);
    menu_append_exit(&measure_menu, "Discard");
    menu_append_submenu(&measure_menu, "Main   menu", &main_menu);
    // run menus
    show_menu(&measure_menu);
}



//void measure(int32_t a) {
//    CEXCEPTION_T e;
//    measure_exit = false;
//    while (true) {
//        Try {
//            measure_get_reading(&measure_orientation);
//        }
//        Catch (e) {
//            if (e==ERROR_LASER_READ_FAILED) {
//                display_on(true);
//                laser_on(false);
//                display_clear_screen(true);
//                display_write_text(0, 0, "Laser read", &large_font, false, true);
//                display_write_text(4, 0, "failed", &large_font, false, true);
//                delay_ms_safe(3000);
//                continue;
//            } else {
//                Throw(e);
//            }
//        }
//        if (measure_exit) break;
//        measure_show_reading(&measure_orientation);
//        if (measure_exit) break;
//    }        
//    laser_on(false);
//    display_on(true);
//}

void do_reading() {
    CEXCEPTION_T e;
    Try {
        get_reading(&measure_orientation);
        beep_beep();
    }
    Catch (e) {
        if (e==ERROR_LASER_READ_FAILED) {
            display_on(true);
            laser_on(false);
            display_clear_screen(true);
            display_write_text(0, 0, "Laser read", &large_font, false, true);
            display_write_text(4, 0, "failed", &large_font, false, true);
            beep_sad();
            delay_ms_safe(1000);
            return;
        } else if (e==ERROR_LASER_TIMEOUT) {
            display_on(true);
            laser_on(false);
            display_clear_screen(true);
            display_write_text(0, 0, "Laser read", &large_font, false, true);
            display_write_text(4, 0, "timed out", &large_font, false, true);
            beep_sad();
            delay_ms_safe(1000);
            return;
        } else {
            Throw(e);
        }
    }
    measure_show_reading(&measure_orientation);    
}

void ready_to_measure() {
    display_on();
    display_clear_screen(true);
    display_rle_image(image_laser2);
    laser_on();    
}

void measure() {
    ready_to_measure();
    while (true) {
        wdt_clear();
        show_status();
        if (measure_requested) {
            measure_requested = false;
            delay_ms_safe(2000);
            do_reading();
            ready_to_measure();
            continue;
        }
        switch (get_input()) {
            case SINGLE_CLICK:
                show_menu(&main_menu);
                ready_to_measure();
                break;
            case LONG_CLICK:
                do_reading();
                ready_to_measure();
                break;
            case DOUBLE_CLICK:
                utils_turn_off(0);
                break;
            default:
                break;
        }
        delay_ms_safe(10);
    }
}