#ifndef MATHS_H
#define MATHS_H
#include <stdint.h>
#include <stdfix.h>
struct EIGEN {
	double scalar;
	double vector[2];
};
/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(accum *a, accum *b, accum *c);

/* returns vector A multiplied by Matrix B in vector C, where A and B are pointers to double[3]
 * and B is a pointer to double[16] */
void apply_matrix(accum *a, accum *b, accum *c);

void normalise(accum *a);

int16_t find_median(int16_t array[],int16_t len);

void pca(accum data[][2], int16_t len, struct EIGEN *eig);
#endif