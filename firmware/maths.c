#include <math.h>
#include <string.h>
#include <gsl_math.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_multilarge.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>
#include "gsl_static.h"
#include "maths.h"
#include "display.h"
#include "calibrate.h"
#include "eigen3x3.h"
/* return AxB in C, where A B and C are all pointers to double[3] */

GSL_MATRIX_DECLARE(lsq_input, CALIBRATION_SAMPLES, 9);
GSL_VECTOR_DECLARE(lsq_output, CALIBRATION_SAMPLES);
GSL_VECTOR_DECLARE(lsq_res, 9);
GSL_MATRIX_DECLARE(lsq_cov, 9, 9);
GSL_MULTIFIT_LINEAR_DECLARE(lsq_workspace, CALIBRATION_SAMPLES, 9);


const matrixx identity = {
    {1.0, 0, 0, 0},
    {0, 1.0, 0, 0},
    {0, 0, 1.0, 0}
};

double amax(const double a, const double b) {
    return a > b ? a : b;
}

double amin(const double a, const double b) {
    return a < b ? a : b;
}

double aabs(const double a) {
    return (a < 0.0) ? (double)(0.0 - a) : (a);
}



void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c) {
    int i, j, k;
    for (i=0; i<3; i++) {
        j = (i+1) % 3;
        k = (i+2) % 3;
        gsl_vector_set(c, i, gsl_vector_get(a, j) * gsl_vector_get(b, k) - gsl_vector_get(a, k) * gsl_vector_get(b, j));
    }
}

/* returns b . a where a is a vector and b is a matrix. Result in vector C, where A and C are pointers to double[3]
 * and B is a pointer to double[16] */
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

double distance2(const double *a, const double *b) {
    double x, y, z;
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

void apply_offset(const double x, const double y, const double z, matrixx matrix) {
    matrixx new_mat = {
        {1.0, 0, 0, x},
        {0, 1.0, 0, y},
        {0, 0, 1.0, z}
    };
    matrix_multiply(matrix, new_mat);
}

void apply_2d_rotation(const int axes[2], const double vector[2], matrixx matrix) {
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

void apply_scale(const int axis, const double scale, matrixx matrix) {
    matrixx new_mat;
    memcpy(new_mat, identity, sizeof(matrixx));
    new_mat[axis][axis] = scale;
    matrix_multiply(matrix, new_mat);    
}

void maths_get_orientation(const gsl_vector *magnetism,
                           const gsl_vector *acceleration,
                           gsl_vector *orientation)  {
    GSL_VECTOR_DECLARE(east, 3);
    GSL_VECTOR_DECLARE(north, 3);
    GSL_VECTOR_DECLARE(down, 3);
    gsl_vector_memcpy(&down, acceleration);
    normalise(&down);
    cross_product(&down, magnetism, &east);
    normalise(&east);
    cross_product(&east, &down, &north);
    normalise(&north);
    gsl_vector_set(orientation, 0, gsl_vector_get(&east, 1));
    gsl_vector_set(orientation, 1, gsl_vector_get(&north, 1));
    gsl_vector_set(orientation, 2, -gsl_vector_get(&down, 1));
    normalise(orientation);
}



void normalise(gsl_vector *vector) {
    gsl_vector_scale(vector, 1.0/ gsl_blas_dnrm2(vector));
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

void get_rotation_matrix(const int axes[2], double theta, matrixx matrix) {
    double v[2];
    memcpy(matrix, identity, sizeof(matrixx));
    v[0] = cos(theta);
    v[1] = sin(theta);
    apply_2d_rotation(axes, v, matrix);
}


/* find the long axis and ratio of long:short axis for an ellipse *
 * data is a set of vectorrs, axes hold the two axes of interest *
 * len is number of data points */
struct ELLIPSE_PARAM 
find_rotation_and_scale_of_ellipse(vectorr *data, 
                                   const int axes[2], 
                                   const int16_t len,
                                   int precision) {
    double maxx;
    double maxy;
    double theta;
    vectorr v;
    matrixx rotation;
    int i,j;
    int a0,a1;
    struct ELLIPSE_PARAM final = {0, 0};
    a0 = axes[0];
    a1 = axes[1];
    for (i=0; i < precision; i++) {
        theta = i * M_PI/precision;
        get_rotation_matrix(axes, theta, rotation);
        maxx = 0;
        maxy = 0;
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

void find_plane(vectorr *data,
                const int axes[2],
                const int16_t len,
                vectorr result) {
    int i;
    double fit, sfit;
    gsl_multilarge_linear_workspace *workspace = gsl_multilarge_linear_alloc(
            gsl_multilarge_linear_normal, 3);
    
    //allocate variables
    GSL_MATRIX_RESIZE(lsq_input, len, 3);
    GSL_VECTOR_RESIZE(lsq_output, len);
    GSL_VECTOR_RESIZE(lsq_res, 3);
    

    //initialise variables
    gsl_vector_set_all(&lsq_output, 1.0);
    for (i=0; i< len; i++) {
        gsl_matrix_set(&lsq_input,i, 0,(double) data[i][0]);
        gsl_matrix_set(&lsq_input,i, 1,(double) data[i][1]);
        gsl_matrix_set(&lsq_input,i, 2,(double) data[i][2]);
    }
    
    //calculate plane
    gsl_multilarge_linear_accumulate(&lsq_input, &lsq_output, workspace);
    gsl_multilarge_linear_solve(0.0, &lsq_res, &fit, &sfit, workspace);
    
    //gsl_multifit_linear(&lsq_input, &lsq_output, &lsq_res, &lsq_cov, &fit, &lsq_workspace);
    
    //process results
    normalise(&lsq_res);
    result[0] = gsl_vector_get(&lsq_res,0);
    result[1] = gsl_vector_get(&lsq_res,1);
    result[2] = gsl_vector_get(&lsq_res,2);
    gsl_multilarge_linear_free(workspace);
}

void sqrtm(gsl_matrix *a, gsl_matrix *result) {
    gsl_vector_view eigenvalues;
    GSL_MATRIX_DECLARE(eigenvectors, 3, 3);
    GSL_MATRIX_DECLARE(t1, 3, 3);
    GSL_MATRIX_DECLARE(t2, 3, 3);
    int i;
    double temp;
    
    gsl_matrix_set_zero(&t1);
    eigenvalues = gsl_matrix_diagonal(&t1);
    eigen3x3(a, &eigenvectors, &eigenvalues.vector);
    
    //square root eigenvalues
    for (i=0; i<3; i++) {
        temp = gsl_vector_get(&eigenvalues.vector, i);
        temp = sqrt(temp);
        gsl_vector_set(&eigenvalues.vector, i, temp);
    };
    
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &eigenvectors, &t1, 0.0, &t2);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, &t2, &eigenvectors, 0.0, result);
    
}

static void make_ellipsoid_matrix(gsl_matrix *ellipsoid, gsl_vector *params) {
    int i, j, index;
    for (i=0; i<3; i++) {
        gsl_matrix_set(ellipsoid, i, i, gsl_vector_get(params, i));
    }
    gsl_matrix_set(ellipsoid, 3, 3, -1.0);
    index = 3;
    for (i=0; i< 3; i++) {
        for (j = i+1; j<3; j++) {
            gsl_matrix_set(ellipsoid, i, j, gsl_vector_get(params, index));
            gsl_matrix_set(ellipsoid, j, i, gsl_vector_get(params, index));
            index++;
        }
    }
    for (i=0; i<3; i++) {
        gsl_matrix_set(ellipsoid, i, 3, gsl_vector_get(params, i+6));
        gsl_matrix_set(ellipsoid, 3, i, gsl_vector_get(params, i+6));        
    }
}

static void get_centre_coords(gsl_matrix *ellipsoid, gsl_vector *centre) {
    int i;
    GSL_MATRIX_DECLARE(a,4,4);
    gsl_matrix_view submat = gsl_matrix_submatrix(&a, 0, 0, 3, 3);
    gsl_vector_view vghi = gsl_matrix_subcolumn(&a, 3, 0, 3);
    gsl_permutation *perm = gsl_permutation_alloc(3);

    gsl_matrix_memcpy(&a, ellipsoid);
    gsl_vector_scale(&vghi.vector, -1.0);
    gsl_linalg_LU_decomp(&submat.matrix, perm, &i);
    gsl_linalg_LU_solve(&submat.matrix, perm, &vghi.vector, centre);
    
    gsl_permutation_free(perm);
}


void calibrate(const double *data_array, const int len, matrixx result) {
    double fit, sfit;
    int i,j;
    matrixx ellipsoid;
    GSL_MATRIX_DECLARE(a4, 4, 4);
    gsl_matrix_view a3 = gsl_matrix_submatrix(&a4,0,0,3,3);
    GSL_MATRIX_DECLARE(b4, 4, 4);
    gsl_matrix_view b3 = gsl_matrix_submatrix(&b4,0,0,3,3);
    GSL_MATRIX_DECLARE(T, 4, 4);
    GSL_VECTOR_DECLARE(centre, 3);
    gsl_vector_view xsq, ysq, zsq, xy2, xz2, yz2, x2, y2, z2;

    //setup GSL variables and aliases...
    gsl_matrix_const_view data = gsl_matrix_const_view_array(data_array, len, 3);
    gsl_vector_const_view x = gsl_matrix_const_column(&data.matrix, 0);
    gsl_vector_const_view y = gsl_matrix_const_column(&data.matrix, 1);
    gsl_vector_const_view z = gsl_matrix_const_column(&data.matrix, 2);
    gsl_multilarge_linear_workspace *workspace;
    gsl_permutation *perm;

    GSL_MATRIX_RESIZE(lsq_input, len, 9);
    GSL_VECTOR_RESIZE(lsq_output, len);
    GSL_VECTOR_RESIZE(lsq_res, 9);
    
    xsq = gsl_matrix_column(&lsq_input, 0);
    ysq = gsl_matrix_column(&lsq_input, 1);
    zsq = gsl_matrix_column(&lsq_input, 2);
    xy2 = gsl_matrix_column(&lsq_input, 3);
    xz2 = gsl_matrix_column(&lsq_input, 4);
    yz2 = gsl_matrix_column(&lsq_input, 5);
    x2  = gsl_matrix_column(&lsq_input, 6);
    y2  = gsl_matrix_column(&lsq_input, 7);
    z2  = gsl_matrix_column(&lsq_input, 8);
    
    //load data into input array
    gsl_vector_memcpy(&xsq.vector, &x.vector); gsl_vector_mul(&xsq.vector, &x.vector);  // xsq = x*x
    gsl_vector_memcpy(&ysq.vector, &y.vector); gsl_vector_mul(&ysq.vector, &y.vector);  // ysq = y*y
    gsl_vector_memcpy(&zsq.vector, &z.vector); gsl_vector_mul(&zsq.vector, &z.vector);  // zsq = z*z
    
    gsl_vector_memcpy(&xy2.vector, &x.vector); gsl_vector_mul(&xy2.vector, &y.vector); gsl_vector_scale(&xy2.vector, 2.0); // xy2 = x*y*2  
    gsl_vector_memcpy(&xz2.vector, &x.vector); gsl_vector_mul(&xz2.vector, &z.vector); gsl_vector_scale(&xz2.vector, 2.0); // xz2 = x*z*2  
    gsl_vector_memcpy(&yz2.vector, &y.vector); gsl_vector_mul(&yz2.vector, &z.vector); gsl_vector_scale(&yz2.vector, 2.0); // yz2 = y*z*2
    
    gsl_vector_memcpy(&x2.vector, &x.vector); gsl_vector_scale(&x2.vector, 2.0); //x2 = x*2  
    gsl_vector_memcpy(&y2.vector, &y.vector); gsl_vector_scale(&y2.vector, 2.0); //y2 = y*2  
    gsl_vector_memcpy(&z2.vector, &z.vector); gsl_vector_scale(&z2.vector, 2.0); //z2 = z*2
    
    //load data into output array
    gsl_vector_set_all(&lsq_output,1.0);
    
    //perform least_squares analysis
//    gsl_multifit_linear(&lsq_input, &lsq_output, &lsq_res, &lsq_cov, &fit, &lsq_workspace);
    workspace = gsl_multilarge_linear_alloc(gsl_multilarge_linear_normal, 9);
    gsl_multilarge_linear_accumulate(&lsq_input, &lsq_output, workspace);
    gsl_multilarge_linear_solve(0.0, &lsq_res, &fit, &sfit, workspace);
    gsl_multilarge_linear_free(workspace);
    //fill a4
    make_ellipsoid_matrix(&a4, &lsq_res);
    //fill vghi
    get_centre_coords(&a4, &centre);
    memcpy(result, identity, sizeof(identity));
    result[0][3] = -1.0*gsl_vector_get(&centre, 0);
    result[1][3] = -1.0*gsl_vector_get(&centre, 1);
    result[2][3] = -1.0*gsl_vector_get(&centre, 2);
    
    gsl_matrix_set_identity(&T);
    gsl_matrix_set(&T, 3, 0, gsl_vector_get(&centre, 0));
    gsl_matrix_set(&T, 3, 1, gsl_vector_get(&centre, 1));
    gsl_matrix_set(&T, 3, 2, gsl_vector_get(&centre, 2));
    
    //b4 = T.A4.T^T
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &T, &a4, 0.0, &b4);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, &b4, &T, 0.0, &a4);
    gsl_matrix_memcpy(&b4, &a4);
    gsl_matrix_scale(&b3.matrix,-1.0*gsl_matrix_get(&b4,3,3));
    
    //A3 = sqrt(B3)
    sqrtm(&b3.matrix, &a3.matrix);
    //result[0:3,0:3] = a3
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++) {
            ellipsoid[i][j] = gsl_matrix_get(&a3.matrix, i, j);
        }
        ellipsoid[i][3] = 0.0;
    }
    matrix_multiply(result, ellipsoid);
}

