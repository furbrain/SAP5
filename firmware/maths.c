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
#include <gsl/gsl_min.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_statistics.h>
#include "gsl_static.h"
#include "maths.h"
#include "display.h"
#include "calibrate.h"
#include "eigen3x3.h"
/* return AxB in C, where A B and C are all pointers to double[3] */

GSL_MATRIX_DECLARE(lsq_input, CALIBRATION_SAMPLES, 9);
GSL_VECTOR_DECLARE(lsq_output, CALIBRATION_SAMPLES);


void cross_product(const gsl_vector *a, const gsl_vector *b, gsl_vector *c) {
    int i, j, k;
    for (i=0; i<3; i++) {
        j = (i+1) % 3;
        k = (i+2) % 3;
        gsl_vector_set(c, i, gsl_vector_get(a, j) * gsl_vector_get(b, k) - gsl_vector_get(a, k) * gsl_vector_get(b, j));
    }
}

/* create a calibration structure from a data array*/
calibration calibration_from_doubles(double *data) {
    calibration c;
    c.transform = gsl_matrix_view_array(data,3,3);
    c.offset = gsl_vector_view_array(&data[9],3);
    return c;
}


/* copy data from src calibration to dest*/
void calibration_memcpy(calibration *dest, const calibration *src) {
    gsl_matrix_memcpy(&dest->transform.matrix, &src->transform.matrix);
    gsl_vector_memcpy(&dest->offset.vector, &src->offset.vector);
}


/* returns b . a where a is a vector and b is a calibration transform. Result in vector c*/
void apply_calibration(const gsl_vector *a, const calibration *b, gsl_vector *c) {
    GSL_VECTOR_DECLARE(temp,3);
    gsl_vector_memcpy(&temp, a);
    gsl_vector_add(&temp, &b->offset.vector);
    gsl_blas_dgemv(CblasNoTrans, 1.0, &b->transform.matrix, &temp, 0, c);
}

/* take magnetism and acceleration vectors in device coordinates
   and return devices orientation as a rotation matrix */
void maths_get_orientation_as_matrix(const gsl_vector *magnetism,
                                     const gsl_vector *acceleration,
                                     gsl_matrix *orientation) {
    gsl_vector_view east = gsl_matrix_column(orientation, 0);
    gsl_vector_view north = gsl_matrix_column(orientation, 1);
    gsl_vector_view up = gsl_matrix_column(orientation, 2);
    gsl_vector_memcpy(&up.vector, acceleration);
    gsl_vector_scale(&up.vector, -1.0);
    gsl_vector_memcpy(&north.vector, magnetism); //use north as temporary holder for magnetism
    normalise(&up.vector);
    normalise(&north.vector);
    cross_product(&north.vector, &up.vector, &east.vector);
    normalise(&east.vector);
    cross_product(&up.vector, &east.vector, &north.vector);
    normalise(&north.vector);
}

void maths_get_orientation_as_vector(const gsl_vector *magnetism,
                           const gsl_vector *acceleration,
                           gsl_vector *orientation)  {
    GSL_MATRIX_DECLARE(rotation, 3, 3);
    gsl_vector_view answer = gsl_matrix_row(&rotation,1);
    maths_get_orientation_as_matrix(magnetism, acceleration, &rotation);
    gsl_vector_memcpy(orientation, &answer.vector);
}


void normalise(gsl_vector *vector) {
    gsl_vector_scale(vector, 1.0/ gsl_blas_dnrm2(vector));
}

static void solve_least_squares(gsl_matrix *input, gsl_vector *output, int num_params, gsl_vector *results) {
    double fit, sfit;
    gsl_multilarge_linear_workspace *workspace = gsl_multilarge_linear_alloc(gsl_multilarge_linear_normal, num_params);
    gsl_multilarge_linear_accumulate(input, output, workspace);
    gsl_multilarge_linear_solve(0.0, results, &fit, &sfit, workspace);
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

static void prepare_input_matrix(gsl_matrix *input, const gsl_matrix *original_data, int len) {
    //setup GSL variables and aliases...
    gsl_matrix_const_view data = gsl_matrix_const_submatrix(original_data, 0, 0, len, 3);
    gsl_vector_const_view x = gsl_matrix_const_column(&data.matrix, 0);
    gsl_vector_const_view y = gsl_matrix_const_column(&data.matrix, 1);
    gsl_vector_const_view z = gsl_matrix_const_column(&data.matrix, 2);
    gsl_vector_view xsq = gsl_matrix_column(&lsq_input, 0);
    gsl_vector_view ysq = gsl_matrix_column(&lsq_input, 1);
    gsl_vector_view zsq = gsl_matrix_column(&lsq_input, 2);
    gsl_vector_view xy2 = gsl_matrix_column(&lsq_input, 3);
    gsl_vector_view xz2 = gsl_matrix_column(&lsq_input, 4);
    gsl_vector_view yz2 = gsl_matrix_column(&lsq_input, 5);
    gsl_vector_view x2  = gsl_matrix_column(&lsq_input, 6);
    gsl_vector_view y2  = gsl_matrix_column(&lsq_input, 7);
    gsl_vector_view z2  = gsl_matrix_column(&lsq_input, 8);
    
    //copy data into input_matrix
    gsl_vector_memcpy(&xsq.vector, &x.vector); gsl_vector_mul(&xsq.vector, &x.vector);  // xsq = x*x
    gsl_vector_memcpy(&ysq.vector, &y.vector); gsl_vector_mul(&ysq.vector, &y.vector);  // ysq = y*y
    gsl_vector_memcpy(&zsq.vector, &z.vector); gsl_vector_mul(&zsq.vector, &z.vector);  // zsq = z*z
    
    gsl_vector_memcpy(&xy2.vector, &x.vector); gsl_vector_mul(&xy2.vector, &y.vector); gsl_vector_scale(&xy2.vector, 2.0); // xy2 = x*y*2  
    gsl_vector_memcpy(&xz2.vector, &x.vector); gsl_vector_mul(&xz2.vector, &z.vector); gsl_vector_scale(&xz2.vector, 2.0); // xz2 = x*z*2  
    gsl_vector_memcpy(&yz2.vector, &y.vector); gsl_vector_mul(&yz2.vector, &z.vector); gsl_vector_scale(&yz2.vector, 2.0); // yz2 = y*z*2
    
    gsl_vector_memcpy(&x2.vector, &x.vector); gsl_vector_scale(&x2.vector, 2.0); //x2 = x*2  
    gsl_vector_memcpy(&y2.vector, &y.vector); gsl_vector_scale(&y2.vector, 2.0); //y2 = y*2  
    gsl_vector_memcpy(&z2.vector, &z.vector); gsl_vector_scale(&z2.vector, 2.0); //z2 = z*2
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

static void convert_ellipsoid_to_transform(gsl_matrix *ellipsoid, gsl_vector *centre, gsl_matrix *results) {
    GSL_MATRIX_DECLARE(T, 4, 4);
    GSL_MATRIX_DECLARE(temp1, 4, 4);
    GSL_MATRIX_DECLARE(temp2, 4, 4);
    gsl_matrix_view temp2_submat = gsl_matrix_submatrix(&temp2, 0, 0, 3, 3);

    //initialise T
    gsl_matrix_set_identity(&T);
    gsl_matrix_set(&T, 3, 0, gsl_vector_get(centre, 0));
    gsl_matrix_set(&T, 3, 1, gsl_vector_get(centre, 1));
    gsl_matrix_set(&T, 3, 2, gsl_vector_get(centre, 2));

    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &T, ellipsoid, 0.0, &temp1);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, &temp1, &T, 0.0, &temp2);
    gsl_matrix_scale(&temp2_submat.matrix,-1.0 / gsl_matrix_get(&temp2,3,3));
    
    //return sqrt(temp2)
    sqrtm(&temp2_submat.matrix, results);
}

void fit_ellipsoid(const gsl_matrix *data, const int len, calibration *result) {
    GSL_MATRIX_DECLARE(a4, 4, 4);
    GSL_VECTOR_DECLARE(params, 9);

    GSL_MATRIX_RESIZE(lsq_input, len, 9);
    GSL_VECTOR_RESIZE(lsq_output, len);

    prepare_input_matrix(&lsq_input, data, len);
    gsl_vector_set_all(&lsq_output,1.0);
    solve_least_squares(&lsq_input, &lsq_output, 9, &params);
    make_ellipsoid_matrix(&a4, &params);
    get_centre_coords(&a4, &result->offset.vector);
    convert_ellipsoid_to_transform(&a4, &result->offset.vector, &result->transform.matrix);
    gsl_vector_scale(&result->offset.vector,-1);
}


struct alignment_params {
    const gsl_matrix *mag_data;
    const calibration *mag_cal;
    const gsl_matrix *grav_data; 
    const calibration *grav_cal;
};

// apply rotations from angles, specified in xzy order
void rotate_calibration(double angles[3], calibration *cal) {
    const int axes[3][2] = {{0, 1}, {1, 2}, {0, 2}};
    GSL_MATRIX_DECLARE(rotation, 3, 3);
    GSL_MATRIX_DECLARE(temp, 3, 3);
    double c, s;
    int i, j;
    for (i=0; i<3; i++) {
        gsl_matrix_set_identity(&rotation);
        c = cos(angles[i]);
        s = sin(angles[i]);
        for (j=0; j<3; j++) {
            gsl_vector_view column = gsl_matrix_column(&rotation, j);
            gsl_linalg_givens_gv(&column.vector, axes[i][0], axes[i][1], c, s);
        }
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &rotation, &cal->transform.matrix, 0, &temp);
        gsl_matrix_memcpy(&cal->transform.matrix, &temp);    
    }
}

void apply_rotations_to_cals(const gsl_vector *rotations, calibration *mag, calibration *grav) {
    double angles[3];
    angles[0] = gsl_vector_get(rotations, 0);
    angles[1] = gsl_vector_get(rotations, 1);
    angles[2] = 0;
    rotate_calibration(angles, mag);
    angles[0] = gsl_vector_get(rotations, 2);
    angles[1] = gsl_vector_get(rotations, 3);
    angles[2] = 0;
    rotate_calibration(angles, grav);
}

double alignment_value(const gsl_vector *rotations, void *params) {
    struct alignment_params *p = (struct alignment_params *) params;
    CALIBRATION_DECLARE(temp_mag_cal);
    CALIBRATION_DECLARE(temp_grav_cal);
    calibration_memcpy(&temp_mag_cal, p->mag_cal);
    calibration_memcpy(&temp_grav_cal, p->grav_cal);
    //apply rotations
    apply_rotations_to_cals(rotations, &temp_mag_cal, &temp_grav_cal);
    return check_accuracy(p->mag_data, &temp_mag_cal, p->grav_data, &temp_grav_cal);
}

double align_all_sensors(const gsl_matrix *mag_data, calibration *mag_cal,
                    const gsl_matrix *grav_data, calibration *grav_cal) {
    struct alignment_params params = {mag_data, mag_cal, grav_data, grav_cal};
    gsl_multimin_function func = {.f = alignment_value, .n = 4, .params = &params}; 
    gsl_multimin_fminimizer *minimizer;
    GSL_VECTOR_DECLARE(starting_point, 4);
    GSL_VECTOR_DECLARE(step_size, 4);
    int iter_count = 0;
    int status;
    double size;
    double result;
    minimizer = gsl_multimin_fminimizer_alloc(gsl_multimin_fminimizer_nmsimplex2, func.n);
    gsl_vector_set_zero(&starting_point);
    gsl_vector_set_all(&step_size, 0.2);
    gsl_multimin_fminimizer_set(minimizer, &func, &starting_point, &step_size);
    do {
        iter_count++;
        status = gsl_multimin_fminimizer_iterate(minimizer);
        if (status)
            break;
        size = gsl_multimin_fminimizer_size(minimizer);
        status = gsl_multimin_test_size(size, 1e-4);

    } while (status==GSL_CONTINUE && iter_count < 1000);
    
    if (status == GSL_SUCCESS) {
        apply_rotations_to_cals(minimizer->x, mag_cal, grav_cal);
    }
    result = minimizer->fval;
    gsl_multimin_fminimizer_free(minimizer);
    return result;
}