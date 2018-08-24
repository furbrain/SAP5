#include <math.h>
#include <string.h>
#include "maths.h"
/* return AxB in C, where A B and C are all pointers to accum[3] */

matrixx identity = {
    {1.0k, 0, 0, 0},
    {0, 1.0k, 0, 0},
    {0, 0, 1.0k, 0}
};

void cross_product(vectorr a, vectorr b, vectorr c) {
	c[0] = (a[1]*b[2]) - (a[2]*b[1]);
	c[1] = (a[2]*b[0]) - (a[0]*b[2]);
	c[2] = (a[0]*b[1]) - (a[1]*b[0]);
}

/* returns vector A multiplied by Matrix B in vector C, where A and C are pointers to accum[3]
 * and B is a pointer to accum[16] */
void apply_matrix(vectorr a, matrixx b, vectorr c) {
    int i;
    int j;
    for (i=0; i<3; i++){
        c[i] = 0;
        for(j=0; j<3; j++) {
            c[i] += a[j]* b[i][j];
        }
        c[i] += b[i][3];
    }
}

accum distance2(vectorr a, vectorr b) {
    int x, y, z;
    x = a[0]-b[0];
    y = a[1]-b[1];
    z = a[2]-b[2];
    return (x*x)+(y*y)+(z*z);
}

void matrix_multiply(matrixx delta, matrixx calibration) {
    int i,j,k;
    matrixx cal_copy;
    memcpy(cal_copy, calibration, sizeof(matrixx));
    for (i=0; i<3; i++) {
        for (j=0; j<4; j++) {
            calibration[i][j] = 0;
            for (k=0; k<3; k++) {
                calibration[i][j] += delta[i][k]* cal_copy[k][j];
            }
            calibration[i][j] += delta[i][3] * (j==3 ? 1 : 0);
        }
    }
}

void apply_offset(accum x, accum y, accum z, matrixx matrix) {
    matrixx new_mat = {
        {1.0k, 0k, 0k, x},
        {0k, 1.0k, 0k, y},
        {0k, 0k, 1.0k, z}
    };
    matrix_multiply(new_mat, matrix);
}

void apply_2d_rotation(int axes[2], double vector[2], matrixx matrix) {
    matrixx new_mat;
    int x = axes[0];
    int y = axes[1];
    memcpy(new_mat, identity, sizeof(matrixx));
    new_mat[x][x] = (accum) vector[0];
    new_mat[x][y] = (accum) vector[1];
    new_mat[y][x] = (accum) -vector[1];
    new_mat[y][y] = (accum) vector[0];
    matrix_multiply(new_mat, matrix);
}

void apply_scale(int axis, accum scale, matrixx matrix) {
    matrixx new_mat;
    memcpy(new_mat, identity, sizeof(matrixx));
    new_mat[axis][axis] = scale;
    matrix_multiply(new_mat, matrix);    
}


void normalise(accum *a) {
	accum magnitude;
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

void pca(vectorr data[], int axes[2], int16_t len, struct EIGEN *eig){
	double varX,varY,covar;
	double T,D,L1,L2,magnitude;
	int count;
	varX = varY = covar = 0;
	for (count=0; count< len; count++) {
		varX  += data[count][axes[0]]*data[count][axes[0]];
		varY  += data[count][axes[1]]*data[count][axes[1]];
		covar += data[count][axes[0]]*data[count][axes[1]];
	}
	T = varX+varY;
	D = varX*varY-covar*covar;
	L1 = (T/2) + (accum)sqrt((double)(T*T/4-D));
	L2 = (T/2) - (accum)sqrt((double)(T*T/4-D));
	eig->scalar = (accum)sqrt((double)L1)/sqrt((double)L2);
	magnitude = (accum)sqrt((double)((L1-varY)*(L1-varY)+covar*covar));
	eig->vector[0] = (L1-varY)/magnitude;
	eig->vector[1] = covar/magnitude;
}

