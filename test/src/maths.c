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

void apply_calibration_to_matrix(const gsl_matrix *input, const calibration *cal, gsl_matrix *output) {
    size_t i;
    for (i=0; i< input->size1; ++i) {
        gsl_vector_const_view in_row = gsl_matrix_const_row(input, i);
        gsl_vector_view out_row = gsl_matrix_row(output, i);
        apply_calibration(&in_row.vector, cal, &out_row.vector);
    }
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
    normalise(&up.vector);
    cross_product(magnetism, &up.vector, &east.vector);
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

void maths_get_orientation_of_multiple_vectors(const gsl_matrix *magnetism,
        const gsl_matrix *acceleration,
        gsl_matrix *orientation) {
    size_t i;
    for (i=0; i< magnetism->size1; ++i) {
        gsl_vector_const_view mag_row = gsl_matrix_const_row(magnetism, i);
        gsl_vector_const_view grav_row = gsl_matrix_const_row(acceleration, i);
        gsl_vector_view out_row = gsl_matrix_row(orientation, i);
        maths_get_orientation_as_vector(&mag_row.vector, &grav_row.vector, &out_row.vector);
    }    
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


void find_plane(gsl_matrix *input, gsl_vector *result) {
    int len = input->size1;
    GSL_VECTOR_RESIZE(lsq_output, len);
    //initialise variables
    gsl_vector_set_all(&lsq_output, 1.0);
    //calculate plane
    solve_least_squares(input, &lsq_output, 3, result);
    //process results
    normalise(result);
}

/* take the vector describing a plane and produce a rotation that will rotate that
 * plane to be perpindicular to y-axis. Requires the original vector to be close to
 * the y-axis*/
void plane_to_rotation(const gsl_vector *plane, gsl_matrix *rotation) {
    GSL_VECTOR_DECLARE(plane_copy, 3);
    double c, s;
    int i;
    gsl_vector_memcpy(&plane_copy, plane);
    if (gsl_vector_get(plane,1)<0) {
        gsl_vector_scale(&plane_copy, -1);
    };
    gsl_matrix_set_identity(rotation);
    // do z rotation
    gsl_linalg_givens(gsl_vector_get(&plane_copy, 1), gsl_vector_get(&plane_copy, 0), &c, &s);
    for (i=0; i<3; i++) {
        gsl_vector_view column = gsl_matrix_column(rotation, i);
        gsl_linalg_givens_gv(&column.vector, 1, 0, c, s);
    }
    gsl_linalg_givens_gv(&plane_copy, 1, 0, c, s);
    
    // do x rotation
    gsl_linalg_givens(gsl_vector_get(&plane_copy, 1), gsl_vector_get(&plane_copy, 2), &c, &s);
    for (i=0; i<3; i++) {
        gsl_vector_view column = gsl_matrix_column(rotation, i);
        gsl_linalg_givens_gv(&column.vector, 1, 2, c, s);
    }
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

void align_laser(const gsl_matrix *data, calibration *cal) {
    GSL_VECTOR_DECLARE(plane, 3);
    GSL_MATRIX_DECLARE(rotation, 3, 3);
    GSL_MATRIX_DECLARE(temp, 3, 3);
    GSL_MATRIX_DECLARE(fixed_data, CALIBRATION_SAMPLES, 3);
    GSL_MATRIX_RESIZE(fixed_data, data->size1, 3);
    apply_calibration_to_matrix(data, cal, &fixed_data);
    find_plane(&fixed_data, &plane);
    if (gsl_vector_get(&plane,1) < 0) {
        gsl_vector_scale(&plane, -1);
    }
    plane_to_rotation(&plane, &rotation);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &rotation, &cal->transform.matrix, 0, &temp);
    gsl_matrix_memcpy(&cal->transform.matrix, &temp);
}

struct sync_params {
    const gsl_matrix *mag;
    const gsl_matrix *grav;
};


double sync_sensor_value(double x, void *params) {
    GSL_VECTOR_DECLARE(angles, CALIBRATION_SAMPLES);
    GSL_VECTOR_DECLARE(mag, 3);
    GSL_VECTOR_DECLARE(grav, 3);    
    struct sync_params *p = (struct sync_params*) params;
    GSL_VECTOR_RESIZE(angles, p->mag->size1);
    size_t i;
    double f;
    double c = cos(x);
    double s = sin(x);
    for (i=0; i< angles.size; ++i) {
        gsl_matrix_get_row(&mag, p->mag, i);
        gsl_matrix_get_row(&grav, p->grav, i);
        gsl_vector_scale(&mag, 1/gsl_blas_dnrm2(&mag));
        gsl_vector_scale(&grav, 1/gsl_blas_dnrm2(&grav));
        gsl_linalg_givens_gv(&mag, 0, 2, c, s);
        gsl_blas_ddot(&mag, &grav, &f);
        gsl_vector_set(&angles, i, f);
    }
    return gsl_stats_sd(angles.data, angles.stride, angles.size);
}

double sync_sensors(const gsl_matrix *mag_data, calibration *mag_cal,
                    const gsl_matrix *grav_data, calibration *grav_cal) {
    GSL_MATRIX_DECLARE(fixed_mag, CALIBRATION_SAMPLES, 3);
    GSL_MATRIX_DECLARE(fixed_grav, CALIBRATION_SAMPLES, 3);
    GSL_MATRIX_RESIZE(fixed_mag, mag_data->size1, 3);
    GSL_MATRIX_RESIZE(fixed_grav, grav_data->size1, 3);

    struct sync_params params = {&fixed_mag, &fixed_grav};
    
    gsl_min_fminimizer *minimizer =  gsl_min_fminimizer_alloc(gsl_min_fminimizer_goldensection);
    gsl_function F;
    int iter = 0;
    int status;
    double a, b, result;
    double c, s;
    /* correct current data*/
    apply_calibration_to_matrix(mag_data, mag_cal, &fixed_mag);
    apply_calibration_to_matrix(grav_data, grav_cal, &fixed_grav);
    F.function = sync_sensor_value;
    F.params = &params;
    gsl_min_fminimizer_set(minimizer, &F, 0, -1, 1);
    do {
        iter++;
        status = gsl_min_fminimizer_iterate(minimizer);
        a = gsl_min_fminimizer_x_lower(minimizer);
        b = gsl_min_fminimizer_x_upper(minimizer);
        status = gsl_min_test_interval (a, b, 0.0001, 0.0);
    } while (status == GSL_CONTINUE && iter < 100);
    result = gsl_min_fminimizer_x_minimum(minimizer);
    c = cos(result);
    s = sin(result);
    for (iter=0; iter < 3; iter++) {
        gsl_vector_view column = gsl_matrix_column(&mag_cal->transform.matrix, iter);
        gsl_linalg_givens_gv(&column.vector,0,2,c,s);
    }
    gsl_min_fminimizer_free(minimizer);
    
    return result*180/M_PI;
}

