#ifndef GSL_STATIC_H
#define GSL_STATIC_H
#include <gsl/gsl_block.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>

#ifdef __XC32__
#define PADDING {0},
#else
#define PADDING
#endif

#define GSL_VECTOR_DECLARE(name, max_size) \
    double name##_data[max_size]; \
    gsl_block name##_block = {max_size, name##_data}; \
    gsl_vector name = {max_size, 1, name##_data, &name##_block, 1}

#define GSL_VECTOR_RESIZE(name, vec_size) name.size = vec_size

#define GSL_MATRIX_DECLARE(name, max_a, max_b) \
    double name##_data[max_a*max_b]; \
    gsl_block name##_block = {max_a*max_b, name##_data}; \
    gsl_matrix name = {max_a, max_b, max_b, name##_data, &name##_block, 1}
    
#define GSL_MATRIX_RESIZE(name, a, b) \
    name.size1 = a; \
    name.size2 = b; \
    name.tda = b
    
#define GSL_MULTIFIT_LINEAR_DECLARE(name, nmax, pmax) \
    GSL_MATRIX_DECLARE(name##_A, nmax, pmax); \
    GSL_MATRIX_DECLARE(name##_Q, pmax, pmax); \
    GSL_MATRIX_DECLARE(name##_QSI, pmax, pmax); \
    GSL_VECTOR_DECLARE(name##_S, pmax); \
    GSL_VECTOR_DECLARE(name##_t, nmax); \
    GSL_VECTOR_DECLARE(name##_xt, pmax); \
    GSL_VECTOR_DECLARE(name##_D, pmax); \
    gsl_multifit_linear_workspace name = { \
        PADDING nmax, pmax, 0, 0, \
        &name##_A, &name##_Q, &name##_QSI, \
        &name##_S, &name##_t, &name##_xt, &name##_D, 0.0}

#endif
