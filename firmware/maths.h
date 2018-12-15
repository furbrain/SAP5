#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>
#include <gsl/gsl_matrix.h>

struct __attribute__((aligned(4))) ELLIPSE_PARAM {
	double scale;
	double theta;
};

typedef double vectorr[3];
typedef double matrixx [3][4];

extern const matrixx identity;

double amax(double a, double b);

double amin(double a, double b);

double aabs(double a);

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c);


/* returns square of theh distance between two points*/
double distance2(const double *a, const double *b);

/* returns vector A multiplied by Matrix B in vector C, where A and C are pointers to double[3]
 * and B is a pointer to double[3][4] */
void apply_matrix(const vectorr a, matrixx b, vectorr c);


/* applies matrix delta to calibration */
void matrix_multiply(matrixx delta, matrixx calibration);

/* apply an offset of x,y,z to matrix */
void apply_offset(const double x, const double y, const double z, matrixx matrix );

/* apply a rotation to matrix such that axis[0] rotates to vector (in specified axes) */
void apply_2d_rotation(const int axes[2], const double vector[2], matrixx matrix);

/* create a rotation of theta radians in axes, store the result in matrix */
void get_rotation_matrix(const int axes[2], double theta, matrixx matrix);

/* apply a scale to a specific axis of matrixx */
void apply_scale(const int axis, const double scale, matrixx matrix);

/* take magnetism and acceleration vectors in device coordinates
   and return devices orientation in world coordinates */
void maths_get_orientation(const gsl_vector *magnetism,
                           const gsl_vector *acceleration,
                           gsl_vector *orientation);

/* normalise a vector to unit length n*/
void normalise(gsl_vector *vector);


/* find the long axis and ratio of long:short axis for an ellipse *
 * data is a set of vectorrs, axes hold the two axes of interest *
 * len is number of data points */
struct ELLIPSE_PARAM 
find_rotation_and_scale_of_ellipse(vectorr *data, 
                                   const int axes[2], 
                                   const int16_t len,
                                   int precision);


/* find the vector describing a plane from a set of points *
 * data is a set of vectorrs, axes hold the two axes that should be varying most *
 * len is the number of data points */
void find_plane(double *data_array, int len, gsl_vector *result);
        

/* finds the median value of array. array is modified and sorted by this function */
int16_t find_median(int16_t array[],const int16_t len);

void sqrtm(gsl_matrix *a, gsl_matrix *result);

void calibrate(const double *data, int len, matrixx result);
#endif
