#include <math.h>
#include <string.h>
#include "maths.h"
/* return AxB in C, where A B and C are all pointers to accum[3] */

const matrixx identity = {
    {1.0k, 0, 0, 0},
    {0, 1.0k, 0, 0},
    {0, 0, 1.0k, 0}
};

accum amax(const accum a, const accum b) {
    return a > b ? a : b;
}

accum amin(const accum a, const accum b) {
    return a < b ? a : b;
}

accum aabs(const accum a) {
    return a < 0k ? (0k-a) : (a);
}



void cross_product(const vectorr a, const vectorr b, vectorr c) {
	c[0] = (a[1]*b[2]) - (a[2]*b[1]);
	c[1] = (a[2]*b[0]) - (a[0]*b[2]);
	c[2] = (a[0]*b[1]) - (a[1]*b[0]);
}

/* returns b . a where a is a vector and b is a matrix. Result in vector C, where A and C are pointers to accum[3]
 * and B is a pointer to accum[16] */
void apply_matrix(const vectorr a, matrixx b, vectorr c) {
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

accum distance2(const vectorr a, const vectorr b) {
    accum x, y, z;
    x = a[0]-b[0];
    y = a[1]-b[1];
    z = a[2]-b[2];
    return (x*x)+(y*y)+(z*z);
}

void matrix_multiply(matrixx calibration, matrixx delta) {
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

void apply_offset(const accum x, const accum y, const accum z, matrixx matrix) {
    matrixx new_mat = {
        {1.0k, 0k, 0k, x},
        {0k, 1.0k, 0k, y},
        {0k, 0k, 1.0k, z}
    };
    matrix_multiply(matrix, new_mat);
}

void apply_2d_rotation(const int axes[2], const accum vector[2], matrixx matrix) {
    matrixx new_mat;
    int x = axes[0];
    int y = axes[1];
    memcpy(new_mat, identity, sizeof(new_mat));
    new_mat[x][x] = vector[0];
    new_mat[x][y] = vector[1];
    new_mat[y][x] = -vector[1];
    new_mat[y][y] = vector[0];
    matrix_multiply(matrix, new_mat);
}

void apply_scale(const int axis, const accum scale, matrixx matrix) {
    matrixx new_mat;
    memcpy(new_mat, identity, sizeof(matrixx));
    new_mat[axis][axis] = scale;
    matrix_multiply(matrix, new_mat);    
}


void normalise(accum vector[], int len) {
	accum magnitude = 0;
	int i;
	for (i=0; i< len; i++) {
    	magnitude += vector[i] * vector[i];
    }
    magnitude = sqrt(magnitude);
	for (i=0; i< len; i++) {
    	vector[i] /=magnitude;
    }
}

int16_t find_median(int16_t array[], const int16_t len) {
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

void get_rotation_matrix(const int axes[2], accum theta, matrixx matrix) {
    accum v[2];
    memcpy(matrix, identity, sizeof(matrixx));
    v[0] = cos(theta);
    v[1] = sin(theta);
    apply_2d_rotation(axes, v, matrix);
}


/* find the long axis and ratio of long:short axis for an ellipse *
 * data is a set of vectorrs, axes hold the two axes of interest *
 * len is number of data points */
struct ELLIPSE_PARAM 
find_rotation_and_scale_of_ellipse(const vectorr *data, 
                                   const int axes[2], 
                                   const int16_t len,
                                   int precision) {
    accum maxx;
    accum maxy;
    accum theta;
    vectorr v;
    matrixx rotation;
    int i,j;
    int a0,a1;
    struct ELLIPSE_PARAM final = {0k, 0k};
    a0 = axes[0];
    a1 = axes[1];
    for (i=0; i < precision; i++) {
        theta = i * M_PI/precision;
        get_rotation_matrix(axes, theta, rotation);
        maxx = 0k;
        maxy = 0k;
        for (j=0; j< len; j++) {
            apply_matrix(data[j], rotation, v);
            maxx = amax(aabs(v[a0]),maxx);
            maxy = amax(aabs(v[a1]),maxy);
        }
        //if (1) {
        if ((maxx / maxy) > final.scale) {
            final.scale = maxx /maxy;
            final.theta = theta;
        }
    }
    return final;          
}
