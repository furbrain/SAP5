#ifndef MEASURE_H
#define MEASURE_H
#include <stdbool.h>
#include <gsl/gsl_vector.h>

/* take one or more measurements and store them */
void measure();
extern bool measure_requested;

/* calculate compass and inclination from an orientation */
void measure_calculate_bearings(gsl_vector *orientation, double *compass, double *inclination);

/* expose functions for testing purpose */
#ifdef TEST
#include "menu.h"

/* calculate extension for this reading */
double get_extension(gsl_vector *orientation);

/* calculate total distance of this reading */
double get_distance(gsl_vector *orientation);

/* add a set of polar entries to a menu */
void add_polar_entries_to_menu(struct menu *menu);

/* add a set of cartesian entries to a menu */
void add_cartesian_entries_to_menu(struct menu *menu);

#endif
#endif
