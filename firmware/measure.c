#include "config.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include "measure.h"
#include "interface.h"
#include "display.h"
#include "sensors.h"
#include "maths.h"
#include "laser.h"
#include "menu.h"
#include "utils.h"
#include "survey.h"
#include "leg.h"
#include "gsl_static.h"

#define FEET_PER_METRE 3.281
#define DEGREES_PER_RADIAN 57.296
#define GRADS_PER_DEGREE 1.111111111
#define NUM_SENSOR_READINGS 10

GSL_VECTOR_DECLARE(measure_orientation, 3);

static bool measure_exit;

DECLARE_EMPTY_MENU(measure_menu, 12);
DECLARE_EMPTY_MENU(storage_menu, 12);

void get_reading(gsl_vector *orientation){
    GSL_VECTOR_DECLARE(magnetism, 3);
    GSL_VECTOR_DECLARE(acceleration, 3);
	struct COOKED_SENSORS sensors;
	double distance;
    gsl_vector_view sensors_magnetism = gsl_vector_view_array(sensors.mag, 3);    
    gsl_vector_view sensors_acceleration = gsl_vector_view_array(sensors.accel, 3);    
	int i;

	display_on(false);
	laser_on(true);
	delay_ms_safe(20);
	gsl_vector_set_zero(&magnetism);
	gsl_vector_set_zero(&acceleration);
	for(i=0; i < NUM_SENSOR_READINGS; ++i) {
		delay_ms_safe(10);
        sensors_read_cooked(&sensors);
        gsl_vector_add(&magnetism, &sensors_magnetism.vector);
        gsl_vector_add(&acceleration, &sensors_acceleration.vector);
    }
    gsl_vector_scale(&magnetism, 1.0/NUM_SENSOR_READINGS);
    gsl_vector_scale(&acceleration, 1.0/NUM_SENSOR_READINGS);
    distance = laser_read(LASER_MEDIUM, 1000);
    maths_get_orientation(&magnetism, &acceleration, orientation);
    gsl_vector_scale(orientation, distance);
    display_on(true);
    laser_on(false);
}

static
void do_exit(int32_t a) {
    measure_exit = true;
}

void measure_get_reading(gsl_vector *orientation) {
    display_clear_screen();
    display_write_text(2, 0, "---*", &large_font,false);
    laser_on(true);
    while (true) {
 		switch(get_action()) {
			case SINGLE_CLICK:
			case LONG_CLICK:
				/* take measurement */
				get_reading(orientation);
				return;
				break;
			case DOUBLE_CLICK:
			    do_exit(0);
				return;
				break;
	        default:
	    delay_ms_safe(10); 
        }
    }
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
TESTABLE_STATIC
void calculate_bearings(gsl_vector *orientation, double *compass, double *inclination){
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
    char text[30];
    calculate_bearings(orientation, &compass, &inclination);

    sprintf(text, "%05.1f`", compass * degree_scale);
    menu_append_info(menu, text);

    sprintf(text, "%+.1f`", inclination * degree_scale);
    menu_append_info(menu, text);

    sprintf(text, "Dist  %.2f", get_distance(orientation) * length_scale);
    menu_append_info(menu, text);

    sprintf(text, "Ext  %.2f", get_extension(orientation) * length_scale);
    menu_append_info(menu, text);
}

/* add a set of cartesian entries to a menu */
TESTABLE_STATIC
void add_cartesian_entries_to_menu(gsl_vector *orientation, struct menu *menu) {
    const char *format[] = {"E: %+.2f", "N: %+.2f","V: %+.2f"};
    double length_scale = (config.length_units==IMPERIAL) ? FEET_PER_METRE : 1.0;
    char text[30];
    int i;
    for (i=0; i<3; i++) {
        sprintf(text, format[i], gsl_vector_get(orientation, i));
        menu_append_info(menu, text);
    }
    sprintf(text, "Ext  %.2f", get_extension(orientation) * length_scale);
    menu_append_info(menu, text);
}


static
void store_leg(int32_t code) {
    struct LEG leg;
    leg = leg_create(utils_get_time(), survey_current.number, 0, 0, &measure_orientation);
    leg_stations_decode(code, &leg.from, &leg.to);
    leg_save(&leg);
}

static
void add_storage_menu_entry(struct menu *menu, uint8_t from, uint8_t to) {
    const char *text;
    int32_t code;
    text = leg_stations_to_text(from, to);
    code = leg_stations_encode(from, to);
    menu_append_action(menu, text, store_leg, code);
}

TESTABLE_STATIC
void setup_storage_menu(void) {
    int last = survey_current.max_station;
    char text[30];
    menu_clear(&storage_menu);
    add_storage_menu_entry(&storage_menu, last, last+1);
    add_storage_menu_entry(&storage_menu, last, LEG_SPLAY);
    add_storage_menu_entry(&storage_menu, last+1, last);
    add_storage_menu_entry(&storage_menu, last+1, LEG_SPLAY);
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
    menu_append_action(&measure_menu, "Main   menu", do_exit, 0);
    // run menus
    show_menu(&measure_menu);
}



void measure(int32_t a) {
    measure_exit = false;
    while (true) {
        measure_get_reading(&measure_orientation);
        if (measure_exit) break;
        measure_show_reading(&measure_orientation);
        if (measure_exit) break;
    }        
    laser_on(false);
    display_on(true);
}
