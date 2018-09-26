#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>
struct EIGEN {
	double scalar;
	double vector[2];
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

void apply_offset(const accum x, const accum y, const accum z, matrixx matrix );

void apply_2d_rotation(const int axes[2], const double vector[2], matrixx matrix);

void apply_scale(const int axis, const accum scale, matrixx matrix);

void normalise(vectorr a);

/* finds the median value of array. array is modified and sorted by this function */
int16_t find_median(int16_t array[],const int16_t len);

void pca(const vectorr data[], const int axes[2], const int16_t len, struct EIGEN *eig);
#endif
