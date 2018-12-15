#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>
#include <gsl/gsl_matrix.h>

typedef double vectorr[3];
typedef double matrixx [3][4];

extern const matrixx identity;

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c);

/* returns vector A multiplied by Matrix B in vector C, where A and C are pointers to double[3]
 * and B is a pointer to double[3][4] */
void apply_matrix(const vectorr a, matrixx b, vectorr c);


/* applies matrix delta to calibration */
void matrix_multiply(matrixx delta, matrixx calibration);

/* take magnetism and acceleration vectors in device coordinates
   and return devices orientation in world coordinates */
void maths_get_orientation(const gsl_vector *magnetism,
                           const gsl_vector *acceleration,
                           gsl_vector *orientation);

/* normalise a vector to unit length n*/
void normalise(gsl_vector *vector);

/* find the vector describing a plane from a set of points *
 * data is a set of vectorrs, axes hold the two axes that should be varying most *
 * len is the number of data points */
void find_plane(double *data_array, int len, gsl_vector *result);
        
void sqrtm(gsl_matrix *a, gsl_matrix *result);

void calibrate(const double *data, int len, matrixx result);
#endif
