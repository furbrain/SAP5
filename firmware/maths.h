#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

typedef struct {
    gsl_matrix_view transform;
    gsl_vector_view offset;
} calibration;

#define CALIBRATION_DECLARE(name) \
    double name##_transform[9]; \
    double name##_offset[3]; \
    calibration name = {gsl_matrix_view_array(name##_transform,3,3),\
                        gsl_vector_view_array(name##_offset,3)};

/* create a calibration structure from a data array*/
calibration calibration_from_doubles(double *data);

/* copy data from src calibration to dest*/
void calibration_memcpy(calibration *dest, const calibration *src);

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c);


/* returns b . a where a is a vector and b is a calibration transform. Result in vector c*/
void apply_calibration(const gsl_vector *a, const calibration *b, gsl_vector *c);

/* applies calibration cal to input, stores result in output*/
void apply_calibration_to_matrix(const gsl_matrix *input, const calibration *cal, gsl_matrix *output);


/* take magnetism and acceleration vectors in device coordinates
   and return devices orientation in world coordinates */
void maths_get_orientation_as_vector(const gsl_vector *magnetism,
        const gsl_vector *acceleration,
        gsl_vector *orientation);

/* take magnetism and acceleration vectors in device coordinates
   and return devices orientation as a rotation matrix */
void maths_get_orientation_as_matrix(const gsl_vector *magnetism,
        const gsl_vector *acceleration,
        gsl_matrix *orientation);

/* normalise a vector to unit length n*/
void normalise(gsl_vector *vector);

/* find the vector describing a plane from a set of points *
 * data is a set of vectorrs, axes hold the two axes that should be varying most *
 * len is the number of data points */
void find_plane(gsl_matrix *input, gsl_vector *result);

/* take the vector describing a plane and produce a rotation that will rotate that
 * plane to be perpindicular to y-axis. Requires the original vector to be close to
 * the y-axis*/
void plane_to_rotation(const gsl_vector *plane, gsl_matrix *rotation);


void sqrtm(gsl_matrix *a, gsl_matrix *result);

void calibrate(const gsl_matrix *data, int len, calibration *result);

void align_laser(const gsl_matrix *data, calibration *cal);

#endif
