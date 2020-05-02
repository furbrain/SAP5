#ifndef MEASURE_H
#define MEASURE_H
#include <stdbool.h>
#include <gsl/gsl_vector.h>

/* take one or more measurements and store them */
void measure();
extern bool measure_requested;

/* calculate compass and inclination from an orientation */
void measure_calculate_bearings(gsl_vector *orientation, double *compass, double *inclination);
union display_strings {
    struct {
        char compass[8];
        char clino[8];
        char distance[8];
        char extension[8];
    };
    struct {
        char north[8];
        char east[8];
        char vertical[8];
        char extension2[8];
    };
    char raw_text[4][8];
};

/* expose functions for testing purpose */
#ifdef TEST
#include "menu.h"

extern union display_strings texts;

/* calculate extension for this reading */
double get_extension(gsl_vector *orientation);

/* calculate total distance of this reading */
double get_distance(gsl_vector *orientation);

/* add a set of polar entries to text strings*/
void fill_polar_strings();

/* add a set of cartesian entries to text strings*/
void fill_cartesian_strings();

/* put polar readings on screen in small font */
void display_polar();

/* put cartesian readings on screen in small font */
void display_cartesian();

/* add a set of polar entries to a menu */
void add_polar_entries_to_menu(struct menu *menu);

/* add a set of cartesian entries to a menu */
void add_cartesian_entries_to_menu(struct menu *menu);

#endif
#endif
