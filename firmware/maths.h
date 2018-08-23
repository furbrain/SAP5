#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>
struct EIGEN {
	double scalar;
	double vector[2];
};

typedef accum vectorr[3];
typedef accum matrixx[3][4];

extern matrixx identity;

/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(vectorr a, vectorr b, vectorr c);

/* returns vector A multiplied by Matrix B in vector C, where A and C are pointers to accum[3]
 * and B is a pointer to accum[3][4] */

void apply_matrix(vectorr a, matrixx b, vectorr c);

void matrix_multiply(matrixx delta, matrixx calibration);

void apply_offset(accum x, accum y, accum z, matrixx matrix );

void apply_2d_rotation(int axes[2], accum vector[2], matrixx matrix);

void normalise(vectorr a);

int16_t find_median(int16_t array[],int16_t len);

void pca(accum data[][2], int16_t len, struct EIGEN *eig);
#endif