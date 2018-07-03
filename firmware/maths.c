#include <math.h>
#include "maths.h"
/* return AxB in C, where A B and C are all pointers to double[3] */
void cross_product(double *a, double *b, double *c) {
	c[0] = (a[1]*b[2]) - (a[2]*b[1]);
	c[1] = (a[2]*b[0]) - (a[0]*b[2]);
	c[2] = (a[0]*b[1]) - (a[1]*b[0]);
}

/* returns vector A multiplied by Matrix B in vector C, where A and B are pointers to double[3]
 * and B is a pointer to double[16] */
void apply_matrix(double *a, double *b, double *c) {
}

void normalise(double *a) {
	double magnitude;
	magnitude = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	a[0] /= magnitude;
	a[1] /= magnitude;
	a[2] /= magnitude;
	
}

int16_t find_median(int16_t array[],int16_t len) {
	int16_t swap;
	int c,d;
	for (c = 0 ; c < ( len - 1 ); c++) {
		for (d = 0 ; d < len - c - 1; d++) {
			if (array[d] > array[d+1]) {
				swap       = array[d];
				array[d]   = array[d+1];
				array[d+1] = swap;
			}
		}
	}
	return array[len/2];
}

void pca(double data[][2], int16_t len, struct EIGEN *eig){
	double varX,varY,covar;
	double T,D,L1,L2,magnitude;
	int count;
	varX = varY = covar = 0;
	for (count=0; count< len; count++) {
		varX  += data[count][0]*data[count][0];
		varY  += data[count][1]*data[count][1];
		covar += data[count][0]*data[count][1];
	}
	T = varX+varY;
	D = varX*varY-covar*covar;
	L1 = (T/2) + sqrt(T*T/4-D);
	L2 = (T/2) - sqrt(T*T/4-D);
	eig->scalar = sqrt(L1)/sqrt(L2);
	magnitude = sqrt((L1-varY)*(L1-varY)+covar*covar);
	eig->vector[0] = (L1-varY)/magnitude;
	eig->vector[1] = covar/magnitude;
}


