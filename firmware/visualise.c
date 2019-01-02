#include <gsl/gsl_blas.h>
#include "visualise.h"
#include "utils.h"
#include "display.h"
#include "gsl_static.h"
#include "maths.h"
#include "interface.h"
#include "sensors.h"



/* convert xyz world coords to xy device coords*/
TESTABLE_STATIC
void convert_to_device(const gsl_matrix *orientation, 
                       const double* coords, 
                       double *x,
                       double *y){
    gsl_vector_const_view world = gsl_vector_const_view_array(coords, 3);
    GSL_VECTOR_DECLARE(device, 3);
    gsl_blas_dgemv(CblasNoTrans, 1.0, orientation, &world.vector, 0.0, &device);
    *x = gsl_vector_get(&device, 0);
    *y = gsl_vector_get(&device, 1);
}

/* given an orientation, calculate the scale and offset
 * required to convert a transformed station to xy screen coordinates*/
TESTABLE_STATIC
void get_offset_and_scale(const struct MODEL_CAVE *cave, 
                          struct VISUALISE_TRANSFORMATION *transform){
    double x, y;
    double maxx = -DBL_MAX;
    double maxy = -DBL_MAX;
    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double scale_x, scale_y;
    int i;
    for (i=0; i< cave->station_count; ++i) {
        convert_to_device(transform->orientation, cave->stations[i].pos, &x, &y);
        maxx = fmax(maxx,x);
        maxy = fmax(maxy,y);
        minx = fmin(minx,x);
        miny = fmin(miny,y);
    }
    transform->offset[0] = -(maxx+minx)/2.0;
    transform->offset[1] = -(maxy+miny)/2.0;
    scale_x = DISPLAY_WIDTH / (maxx-minx);
    scale_y = DISPLAY_HEIGHT / (maxy-miny);
    transform->scale = fmin(scale_x, scale_y);
}

/* convert a station in xyz real-world coordinates to xy screen coordinates*/
TESTABLE_STATIC
void translate_station(const struct MODEL_STATION *station,
                       const struct VISUALISE_TRANSFORMATION *transform,
                       int *x,
                       int *y){
    double dx, dy;
    convert_to_device(transform->orientation, station->pos, &dx, &dy);
    dx += transform->offset[0];
    dy += transform->offset[1];
    dx *= transform->scale;
    dy *= transform->scale;
    *x = (int)(dx + (DISPLAY_WIDTH/2));
    *y = (int)((DISPLAY_HEIGHT/2)-dy);       
}

/* show a cave on the screen, given a current orientation */
TESTABLE_STATIC
void display_cave(const struct MODEL_CAVE *cave,
                  const gsl_matrix *orientation){
    struct VISUALISE_TRANSFORMATION transform;
    int i, x1,x2, y1,y2;
    transform.orientation = orientation;
    get_offset_and_scale(cave, &transform);
    display_clear_screen(false);
    for (i=0; i < cave->leg_count; ++i) {
        translate_station(cave->legs[i].from, &transform, &x1, &y1);
        translate_station(cave->legs[i].to,   &transform, &x2, &y2);
        display_draw_line(x1,y1,x2,y2);
    }
    display_show_buffer();    
}


/* show a list of surveys held on the device*/
void visualise_show_menu(int32_t a){
    
}

/* display a cave on the screen*/
void visualise_survey(uint16_t survey){
    struct MODEL_CAVE cave;
    GSL_MATRIX_DECLARE(orientation, 3, 3);
    struct COOKED_SENSORS sensors;
    gsl_vector_view magnetism = gsl_vector_view_array(sensors.mag, 3);    
    gsl_vector_view acceleration = gsl_vector_view_array(sensors.accel, 3);    

    model_generate(survey, &cave);
    while (true) {
 		switch(get_action()) {
			case SINGLE_CLICK:
			case LONG_CLICK:
			case DOUBLE_CLICK:
				return;
				break;
	        default:
                delay_ms_safe(10); 
        }
        sensors_read_cooked(&sensors);
        maths_get_orientation_as_matrix(&magnetism.vector, &acceleration.vector, &orientation);
        display_cave(&cave, &orientation);
    }

}
