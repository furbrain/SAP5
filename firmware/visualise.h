#ifndef _VISUALISE_H
#define _VISUALISE_H
#include <gsl/gsl_matrix.h>
#include "model.h"

struct VISUALISE_TRANSFORMATION {
    const gsl_matrix *orientation;
    double offset[2];
    double scale;
};

/* show a list of surveys held on the device*/
void visualise_show_menu(int32_t a);

/* display a cave on the screen*/
void visualise_survey(uint16_t survey);

#ifdef TEST
/* convert xyz world coords to xy device coords*/
void convert_to_device(const gsl_matrix *orientation, 
                       const double* coords, 
                       double *x,
                       double *y);

/* given an orientation, calculate the scale and offset
 * required to convert a transformed station to xy screen coordinates*/
void get_offset_and_scale(const struct MODEL_CAVE *cave, 
                          struct VISUALISE_TRANSFORMATION *transform); 

/* convert a station in xyz real-world coordinates to xy screen coordinates*/
void translate_station(const struct MODEL_STATION *station,
                       const struct VISUALISE_TRANSFORMATION *transform,
                       int *x,
                       int *y);

/* show a cave on the screen, given a current orientation */
void display_cave(const struct MODEL_CAVE *cave,
                  const gsl_matrix *orientation);
#endif
#endif // _VISUALISE_H
