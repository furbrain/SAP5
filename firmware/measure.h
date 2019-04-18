#ifndef MEASURE_H
#define MEASURE_H
/* take one or more measurements and store them */
void measure();
extern bool measure_requested;
/* expose functions for testing purpose */
#ifdef TEST
#include <gsl/gsl_vector.h>
#include "menu.h"

void measure_get_reading(gsl_vector *orientation);

/* calculate extension for this reading */
double get_extension(gsl_vector *orientation);

/* calculate total distance of this reading */
double get_distance(gsl_vector *orientation);

/* calculate compass and inclination from an orientation */
void calculate_bearings(gsl_vector *orientation, double *compass, double *inclination);

/* add a set of polar entries to a menu */
void add_polar_entries_to_menu(gsl_vector *orientation, struct menu *menu);

/* add a set of cartesian entries to a menu */
void add_cartesian_entries_to_menu(gsl_vector *orientation, struct menu *menu);

#endif
#endif
