#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#define CALIBRATION_RBF_COUNT 3
#define CALIBRATION_STORAGE_SIZE (12+3*CALIBRATION_RBF_COUNT)

typedef struct {
    gsl_matrix_view transform;
    gsl_vector_view offset;
    gsl_matrix_view rbf;
} calibration;

#define CALIBRATION_DECLARE(name) \
    double name##_transform[9]; \
    double name##_offset[3]; \
    double name##_rbf[3 * CALIBRATION_RBF_COUNT] = {0}; \
    calibration name = {gsl_matrix_view_array(name##_transform, 3, 3),\
                        gsl_vector_view_array(name##_offset, 3), \
                        gsl_matrix_view_array(name##_rbf, 3, CALIBRATION_RBF_COUNT)};

/* create a calibration structure from a data array*/
calibration calibration_from_doubles(double *data);

/* copy data from src calibration to dest*/
void calibration_memcpy(calibration *dest, const calibration *src);

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c);


/* returns b . a where a is a vector and b is a calibration transform. Result in vector c*/
void apply_calibration(const gsl_vector *a, const calibration *b, gsl_vector *c);


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

void sqrtm(gsl_matrix *a, gsl_matrix *result);

/* find the calibration matrix that makes data best fit into a rotated ellipsoid*/
void fit_ellipsoid(const gsl_matrix *data, calibration *result);


/* minimize the error by rotating magnetic and grav sensors */
double align_all_sensors(const gsl_matrix *mag_data, calibration *mag_cal,
                    const gsl_matrix *grav_data, calibration *grav_cal);

/* minimize the error by applying a non-linear filter to mag_cal*/
double optimize_rbf(const gsl_matrix *mag_data, calibration *mag_cal,
                    const gsl_matrix *grav_data, calibration *grav_cal);

#endif
