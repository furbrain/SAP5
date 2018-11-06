#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>

struct __attribute__((aligned(4))) ELLIPSE_PARAM {
	accum scale;
	accum theta;
};

typedef accum vectorr[3];
typedef accum matrixx [3][4];

extern const matrixx identity;

accum amax(accum a, accum b);

accum amin(accum a, accum b);

accum aabs(accum a);

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(const vectorr a, const vectorr b, vectorr c);


/* returns square of theh distance between two points*/
accum distance2(const vectorr a, const vectorr b);

/* returns vector A multiplied by Matrix B in vector C, where A and C are pointers to accum[3]
 * and B is a pointer to accum[3][4] */
void apply_matrix(const vectorr a, matrixx b, vectorr c);


/* applies matrix delta to calibration */
void matrix_multiply(matrixx delta, matrixx calibration);

/* apply an offset of x,y,z to matrix */
void apply_offset(const accum x, const accum y, const accum z, matrixx matrix );

/* apply a rotation to matrix such that axis[0] rotates to vector (in specified axes) */
void apply_2d_rotation(const int axes[2], const accum vector[2], matrixx matrix);

/* create a rotation of theta radians in axes, store the result in matrix */
void get_rotation_matrix(const int axes[2], accum theta, matrixx matrix);

/* apply a scale to a specific axis of matrixx */
void apply_scale(const int axis, const accum scale, matrixx matrix);

/* normalise a vector of length n*/
void normalise(accum vector[], int len);


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
void find_plane(vectorr *data,
                const int axes[2],
                const int16_t len,
                vectorr result);
        

/* finds the median value of array. array is modified and sorted by this function */
int16_t find_median(int16_t array[],const int16_t len);

#endif
