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
#include "ui.h"
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

bool measure_requested = false;

DECLARE_EMPTY_MENU(measure_menu, 12);
DECLARE_EMPTY_MENU(storage_menu, 12);

union display_strings texts;


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

TESTABLE_STATIC
void fill_polar_strings() {
    double compass, inclination;
    double degree_scale = (config.display_style==GRAD) ? GRADS_PER_DEGREE : 1.0;
    double length_scale = (config.length_units==IMPERIAL) ? FEET_PER_METRE : 1.0;
    char length_unit = (config.length_units==IMPERIAL) ? IMPERIAL_UNIT : METRIC_UNIT;
    char angle_symbol = (config.display_style==GRAD) ? GRAD_SYMBOL : DEGREE_SYMBOL;
    gsl_vector *orientation = sensors_get_last_reading();
    measure_calculate_bearings(orientation, &compass, &inclination);

    sprintf(texts.compass, "%05.1f%c", compass * degree_scale, angle_symbol);
    sprintf(texts.clino, "%+.1f%c", inclination * degree_scale, angle_symbol);
    sprintf(texts.distance, "%.2f%c", get_distance(orientation) * length_scale, length_unit);
    sprintf(texts.extension, "%.2f%c", get_extension(orientation) * length_scale, length_unit);
}

TESTABLE_STATIC
void fill_cartesian_strings() {
    double length_scale = (config.length_units==IMPERIAL) ? FEET_PER_METRE : 1.0;
    char length_unit = (config.length_units==IMPERIAL) ? IMPERIAL_UNIT : METRIC_UNIT;
    gsl_vector *orientation = sensors_get_last_reading();
    int i;
    for (i=0; i<3; i++) {
        sprintf(texts.raw_text[i],"%+.2f%c",gsl_vector_get(orientation, i) * length_scale, length_unit);
    }
    sprintf(texts.extension2, "%.2f%c", get_extension(orientation) * length_scale, length_unit);
}

TESTABLE_STATIC
void display_polar() {
    char temp[80];
    sprintf(temp, "Compass: %7s\n"
                  "Clino:   %7s\n"
                  "Distance:%7s\n"
                  "Extended:%7s\n",
            texts.compass, texts.clino, texts.distance, texts.extension);
    display_write_multiline(0, temp, false);
}

TESTABLE_STATIC
void display_cartesian() {
    char temp[80];
    sprintf(temp, "North:   %7s\n"
                  "East:    %7s\n"
                  "Vertical:%7s\n"
                  "Extended:%7s\n",
            texts.north, texts.east, texts.vertical, texts.extension2);
    display_write_multiline(0, temp, false);
}

/* calculate compass and inclination from an orientation */
void measure_calculate_bearings(gsl_vector *orientation, double *compass, double *inclination){
    *compass = atan2(gsl_vector_get(orientation, 0), gsl_vector_get(orientation, 1)) * DEGREES_PER_RADIAN;
	if (*compass<0) *compass+=360;
	*inclination = atan2(gsl_vector_get(orientation, 2), get_extension(orientation))*DEGREES_PER_RADIAN;
}

/* add a set of polar entries to a menu */
TESTABLE_STATIC
void add_polar_entries_to_menu(struct menu *menu) {
    char text[16];
    fill_polar_strings();
    if (config.compact) {
        menu_append_exit(menu, "", display_polar);
    } else {
        menu_append_exit(menu, texts.compass, NULL);
        menu_append_exit(menu, texts.clino, NULL);

        sprintf(text, "Dist  %s", texts.distance);
        menu_append_exit(menu, text, NULL);

        sprintf(text, "Ext  %s", texts.extension);
        menu_append_exit(menu, text, NULL);
    }
}

/* add a set of cartesian entries to a menu */
TESTABLE_STATIC
void add_cartesian_entries_to_menu(struct menu *menu) {
    const char *format[] = {"E: %s", "N: %s","V: %s"};
    char text[16];
    int i;
    fill_cartesian_strings();
    if (config.compact) {
        menu_append_exit(menu, "", display_cartesian);
    } else {
        for (i=0; i<3; i++) {
            sprintf(text, format[i], texts.raw_text[i]);
            menu_append_exit(menu, text, NULL);
        }
        sprintf(text, "Ext  %s", texts.extension2);
        menu_append_exit(menu, text, NULL);
    }
}



static
void add_storage_menu_entry(struct menu *menu, uint8_t from, uint8_t to) {
    const char *text;
    int32_t code;
    text = leg_stations_to_text(from, to);
    code = leg_stations_encode(from, to);
    menu_append_action(menu, text, leg_create_and_store, code);
}

int idx2pos(int i) {
    return 8 + i*14;
}

int txt2num(char x) {
    return x-'0';
}

void custom_storage(int32_t last) {
    int from  = last -1;
    int to = last;
    struct UI_MULTI_SELECT selector = {
        .offset = 20,
        .text = "00 -> 00",
        .numbers = {
           //max   min   curr  len pos cb
            {to,   1,    from,    2,  0,  NULL }, //from
            {to,   1,    to,    2,  6,  NULL}, //to
        }
    };
    int32_t code;
    sprintf(selector.text, "%02d -> %02d", from, to);
    ui_multi_select(&selector);
    from = selector.numbers[0].current;
    to = selector.numbers[1].current;
    code = leg_stations_encode(from, to);
    leg_create_and_store(code);
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
    menu_append_back(&storage_menu, "Back", NULL);
}

void measure_show_reading() {
    // set up menus
    menu_clear(&measure_menu);
    if (config.display_style==CARTESIAN) {
        add_cartesian_entries_to_menu(&measure_menu);
    } else {
        add_polar_entries_to_menu(&measure_menu);
    }
    setup_storage_menu();
    menu_append_submenu(&measure_menu, "Store", &storage_menu);
    menu_append_exit(&measure_menu, "Discard", NULL);
    menu_append_submenu(&measure_menu, "Main   menu", &main_menu);
    // run menus
    show_menu(&measure_menu);
}

void do_reading() {
    CEXCEPTION_T e;
    Try {
        sensors_get_reading();
        beep_beep();
    }
    Catch (e) {
        if (e==ERROR_LASER_READ_FAILED) {
            display_on(true);
            laser_on(false);
            display_clear(false);
            display_write_text(0, 0, "Laser read", &large_font, false);
            display_write_text(4, 0, "failed", &large_font, false);
            display_show_buffer();
            beep_sad();
            delay_ms_safe(1000);
            return;
        } else if (e==ERROR_LASER_TIMEOUT) {
            display_on(true);
            laser_on(false);
            display_clear(false);
            display_write_text(0, 0, "Laser read", &large_font, false);
            display_write_text(4, 0, "timed out", &large_font, false);
            display_show_buffer();
            beep_sad();
            delay_ms_safe(1000);
            return;
        } else {
            Throw(e);
        }
    }
    measure_show_reading();    
}

void ready_to_measure() {
    display_on();
    display_clear(false);
    display_rle_image(image_laser2);
    display_show_buffer();
    laser_on();    
}

void measure() {
    ready_to_measure();
    while (true) {
        wdt_clear();
        show_status(display_screen);
        display_show_buffer();
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
