#include "unity.h"
#include "gsl_static.h"
#include "gsl_machine.h"

void test_gsl_vector() {
    GSL_VECTOR_DECLARE(vector, 10);
    TEST_ASSERT_EQUAL_INT(10,vector.size); 
    GSL_VECTOR_RESIZE(vector, 8);
    TEST_ASSERT_EQUAL_INT(8,vector.size); 
    TEST_ASSERT_EQUAL_INT(1,vector.stride);
    TEST_ASSERT_NOT_NULL(vector.data);    
    TEST_ASSERT_NOT_NULL(vector.block);
    }
    
void test_gsl_matrix() {
    GSL_MATRIX_DECLARE(matrix, 10,10);
    TEST_ASSERT_EQUAL_INT(10, matrix.size1); 
    TEST_ASSERT_EQUAL_INT(10, matrix.size2); 
    GSL_MATRIX_RESIZE(matrix, 8, 9);
    TEST_ASSERT_EQUAL_INT(8, matrix.size1); 
    TEST_ASSERT_EQUAL_INT(9, matrix.size2); 
    TEST_ASSERT_EQUAL_INT(9, matrix.tda);
    TEST_ASSERT_NOT_NULL(matrix.data);    
    TEST_ASSERT_NOT_NULL(matrix.block);
    }
    
void test_gsl_multifit_linear() {
    GSL_MULTIFIT_LINEAR_DECLARE(w, 10, 5);
    TEST_ASSERT_EQUAL_INT(w.nmax, 10);
    TEST_ASSERT_EQUAL_INT(w.pmax, 5);
    TEST_ASSERT_EQUAL_INT(w.n, 0);
    TEST_ASSERT_EQUAL_INT(w.p, 0);
    TEST_ASSERT_NOT_NULL(w.Q);
    TEST_ASSERT_NOT_NULL(w.QSI);
    TEST_ASSERT_NOT_NULL(w.A);
    TEST_ASSERT_NOT_NULL(w.S);
    TEST_ASSERT_NOT_NULL(w.t);
    TEST_ASSERT_NOT_NULL(w.xt);
    TEST_ASSERT_NOT_NULL(w.D);
    }
    
void test_gsl_precision() {
    double a;
    a = 1.0 + GSL_DBL_EPSILON;
    TEST_ASSERT(a != 1.0);
    a = 1.0 + (GSL_DBL_EPSILON/2);
    TEST_ASSERT(a == 1.0);
}
