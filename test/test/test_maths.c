#include "unity.h"
#include "maths.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics.h>
#include <xc.h>
#include "test_maths_fixtures.inc"
#include "exception.h"
#include "eigen3x3.h"
#include "gsl_static.h"
#include "mag_sample_data.inc"
#include "mock_calibrate.h"

#define DEGREES_PER_RADIAN 57.296

void suiteSetUp(void) {
    exception_init();
}

void test_cross_product(void) {
    char text[24];
    double dictionary[12][3][3] = {
        {{0,0,0}, {0,0,0}, {0,0,0}},
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
        {{1.966021,0.996371,1.456462},{0.950379,0.894036,1.886508},{0.577531,-2.324722,0.810763}},
        {{0.080797,1.369236,0.536854},{1.393359,0.653164,1.249846},{1.360681,0.647046,-1.855063}},
        {{0.487519,0.220578,1.785382},{1.130160,1.245123,1.146483},{-1.970131,1.458834,0.357733}},
        {{1.781504,1.491375,1.293967},{0.491547,1.181605,0.133473},{-1.329900,0.398263,1.371954}},
        {{0.928645,0.048362,0.300580},{0.877071,1.622896,1.308908},{-0.424509,-0.951881,1.464677}},
        {{1.473763,1.755599,0.533648},{0.850665,1.980126,0.610196},{0.014570,-0.445330,1.424811}},
        {{1.866873,0.693270,1.198487},{0.981590,1.248449,1.572356},{-0.406183,-1.758966,1.650190}},
        {{0.245519,1.642427,0.415957},{0.467185,0.636266,1.353699},{1.958693,-0.138030,-0.611103}},
        {{0.893915,1.630456,0.372925},{0.158390,0.384927,1.943209},{3.024769,-1.677997,0.085844}},
        {{0.123774,0.762176,0.541222},{0.779024,0.305382,0.958143},{0.564994,0.303032,-0.555955}},
    };
    double result_data[3];
    gsl_vector_view a, b;
    gsl_vector_view c = gsl_vector_view_array(result_data, 3); 
    int i;
    for (i=0; i<12; i++) {
        a = gsl_vector_view_array(dictionary[i][0], 3);
        b = gsl_vector_view_array(dictionary[i][1], 3);
        cross_product(&a.vector, &b.vector, &c.vector);
        snprintf(text,24,"Iteration: %d",i);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(dictionary[i][2], result_data, 3, text);
    }
}

void test_calibration_from_doubles(void) {
    double data[12] = {1,2,3,4,5,6,7.2,8,9,10,11};
    calibration cal = calibration_from_doubles(data);
    int i,j;
    char text[24];
    for (i=0; i<3; ++i) {
        for (j=0; j<3; ++j) {
            snprintf(text,24,"Transform: %d:%d",i,j);
            TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(data[i*3+j], gsl_matrix_get(&cal.transform.matrix,i,j), text);
        }
        snprintf(text,24,"Offset: %d:%d",i,j);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(data[9+i], gsl_vector_get(&cal.offset.vector,i), text);
        
    }
}

void test_calibration_memcpy(void) {
    double data1[12] = {1,2,3,4,5,6,7.2,8,9,10,11};
    double data2[12] = {0};
    calibration cal1 = calibration_from_doubles(data1);
    calibration cal2 = calibration_from_doubles(data2);
    calibration_memcpy(&cal2, &cal1);
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY(data1,data2,12);
}

void test_apply_calibration(void) {
    char text[24];
    struct test_field {
        double a[3];
        double transform[3][3];
        double offset[3];
        double result[3];
    };
    struct test_field test_cases[20] = {
        {{1.0853,-1.917,0.5346},{{-0.006,-1.1008,-1.2077}, {-1.3236,-1.6466,0.7414}, {-1.9842,0.0488,1.2505}},{0.1702,-1.4313,-0.5066},{3.6446,3.8725,-2.6194}},
        {{-0.2327,-0.2639,0.4711},{{0.6016,0.4042,1.2209}, {1.6346,-0.7231,-1.6382}, {-1.5441,1.3147,-1.8124}},{-0.5934,1.0186,-0.8162},{-0.6133,-1.3306,2.8931}},
        {{-0.698,-1.3399,-0.4299},{{1.2844,-1.3954,-0.4635}, {1.9505,-0.1748,1.3045}, {0.3895,1.6113,0.1382}},{-0.6771,1.0953,-1.8402},{-0.3726,-5.6006,-1.2435}},
        {{-0.7403,0.546,-0.6146},{{1.5197,1.053,1.5124}, {0.4223,0.0539,0.3913}, {-0.7965,-1.8984,-0.7877}},{-1.743,1.9153,-0.6412},{-3.0814,-1.4076,-1.7052}},
        {{1.9083,-0.2369,-0.7269},{{0.3125,1.4157,-1.7276}, {1.1278,0.8744,0.3441}, {-0.5974,0.2528,-0.8011}},{-1.5677,-1.2844,1.5433},{-3.4577,-0.6652,-1.242}},
        {{-1.1249,1.01,-1.5725},{{-0.1209,0.393,-1.4095}, {0.5803,-1.8055,-1.0055}, {-1.0929,-0.4744,1.6889}},{0.2921,1.167,0.2462},{2.8256,-3.0802,-2.3624}},
        {{0.3368,0.8354,-1.4059},{{0.7756,-1.5815,-0.2416}, {0.0279,1.2761,-1.6396}, {0.2605,0.3574,-1.2076}},{0.9795,0.2292,-0.4595},{-0.2122,4.4538,2.976}},
        {{1.353,0.3962,1.1309},{{0.4127,1.1242,0.4629}, {1.0019,-1.2958,-0.1659}, {-0.0639,1.3775,-1.3007}},{-1.5331,-0.6453,-1.6214},{-0.5814,0.2238,0.3064}},
        {{-1.6917,-1.1762,0.2951},{{0.6229,1.2143,-0.5951}, {1.2533,1.1395,-0.4263}, {-0.4639,-0.9708,1.3176}},{1.8638,-0.732,1.4622},{-3.2557,-2.7078,4.0881}},
        {{-1.8989,-1.8032,-1.2615},{{-0.9701,1.6543,-0.1686}, {1.2396,-0.3861,-1.9023}, {-0.9028,0.8364,-0.5769}},{-0.4894,-0.285,0.0448},{-0.9325,0.1602,1.1115}},
        {{-0.7992,-0.4142,1.1731},{{-1.2605,1.361,0.2771}, {1.9557,-1.0849,0.4588}, {1.2111,0.5102,1.2849}},{-0.9464,-1.5107,-0.1651},{-0.1401,-0.863,-1.8011}},
        {{0.7223,-1.227,-1.813},{{1.4346,1.3116,1.8556}, {1.7237,-1.4129,-1.5442}, {1.0662,-0.7154,0.5521}},{-0.4635,-0.4155,1.5891},{-2.1986,3.1126,1.3274}},
        {{-0.3785,-1.4793,-1.6521},{{-0.7817,0.3362,1.5139}, {0.4038,0.5809,-1.7735}, {0.7671,-1.2439,1.212}},{0.1154,0.7852,-1.181},{-4.3166,4.515,-2.7722}},
        {{1.1731,-1.833,1.8534},{{0.2026,-1.7404,-0.6191}, {1.2034,-1.1683,-1.427}, {-1.7682,-0.9736,0.0413}},{-0.8451,0.8832,0.2032},{0.4464,-1.4304,0.4297}},
        {{0.3213,-1.2617,0.462},{{0.0672,0.5046,0.0202}, {-0.3468,0.1419,-0.6294}, {0.6513,1.7427,0.4509}},{-1.4718,1.6302,1.6409},{0.1512,-0.8724,0.8411}},
        {{-1.65,-1.5325,1.0843},{{-1.6515,-0.5701,1.0928}, {0.1513,1.0189,-0.9099}, {-0.0933,0.2265,-0.2371}},{-0.0911,-1.5993,0.4568},{6.3452,-4.8567,-0.9122}},
        {{0.9356,-0.7137,-1.7286},{{0.2365,-1.3566,-0.9284}, {-1.9226,-1.3933,-1.8644}, {-0.5581,1.3019,-0.3511}},{1.4126,0.0882,-0.2289},{3.2214,0.0064,-1.4376}},
        {{0.608,1.1542,1.5691},{{-1.4576,1.0037,0.1073}, {-0.2803,1.3554,0.1494}, {-1.7644,-1.6322,-1.3032}},{0.6071,0.5517,0.0143},{0.1111,2.2081,-6.9918}},
        {{1.2268,1.7141,0.4557},{{0.86,-1.9654,-0.8474}, {-0.2273,-1.3078,-1.5419}, {0.5956,0.007,-0.8574}},{-1.5802,0.0425,0.582},{-4.6356,-3.817,-1.088}},
        {{-1.993,1.8682,1.2315},{{-0.8836,-1.5173,-0.0772}, {1.6597,-1.0245,-0.4206}, {-0.8975,1.7514,1.076}},{1.3527,0.8481,-0.7705},{-3.5912,-4.0394,5.828}},
    };
    GSL_VECTOR_DECLARE(result,3);
    CALIBRATION_DECLARE(cal);
    int i;
    
    for (i=0; i<20; i++) {
        gsl_vector_view src = gsl_vector_view_array(test_cases[i].a, 3);
        memcpy(cal_transform, test_cases[i].transform, sizeof(cal_transform));
        memcpy(cal_offset, test_cases[i].offset, sizeof(cal_offset));
        apply_calibration(&src.vector, &cal, &result);
        
        snprintf(text,24,"Iteration: %d",i);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(test_cases[i].result, result_data, 3, text);
    }
}


void test_maths_get_orientation_as_vector(void) {
    char text[80];
    struct test_field {
        double accel[3];
        double magnetism[3];
        double result[3];
    };
    struct test_field test_cases[10] = {
        {{0.,0.,-1.}, {0,6,-8}, {0.,1.,-0.}},
        {{0.,0.,-1.}, {-6,0,-8}, {1.,0.,-0.}},
        {{0.,1.,0.}, {-6,8,0}, {-0.,0.,-1.}},
        {{1.0853,-1.917,0.5346}, {-0.006,-1.1008,-1.2077}, {0.384,-0.3707,0.8457}},
        {{-1.3236,-1.6466,0.7414}, {-1.9842,0.0488,1.2505}, {0.0467,0.676,0.7354}},
        {{0.887,-0.8325,1.6711}, {0.1702,-1.4313,-0.5066}, {0.2353,-0.8846,0.4028}},
        {{-0.2327,-0.2639,0.4711}, {0.6016,0.4042,1.2209}, {0.7394,0.5018,0.4489}},
        {{1.6346,-0.7231,-1.6382}, {-1.5441,1.3147,-1.8124}, {0.8353,0.4619,0.2982}},
        {{0.1903,1.2771,-1.2042}, {-0.5934,1.0186,-0.8162}, {0.6679,0.1753,-0.7233}},
        {{-0.698,-1.3399,-0.4299}, {1.2844,-1.3954,-0.4635}, {-0.309,-0.4205,0.853}},
    };
    double result_data[3];
    gsl_vector_view accel;
    gsl_vector_view mag;
    gsl_vector_view result = gsl_vector_view_array(result_data,3);
    int i;
    for (i=0; i<10; i++) {
        accel = gsl_vector_view_array(test_cases[i].accel, 3);
        mag = gsl_vector_view_array(test_cases[i].magnetism, 3);
        snprintf(text,24,"Iteration: %d", i);
        maths_get_orientation_as_vector(&mag.vector, &accel.vector, &result.vector);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(test_cases[i].result, result_data, 3, text);
    }
}

void test_normalise(void) {
    char text[80];
    struct test_field {
        double a[3];
        int len;
        double result[3];
    };
    struct test_field test_cases[14] = {
        {{1,0,5}, 1, {1,0,6}},
        {{-1,0,0}, 1, {-1, 0, 0}},
        {{2,0,0}, 1, {1, 0, 0}},
        {{3,4,0}, 2, {0.6, 0.8, 0}},
        {{1.0853,-1.917,0.5346}, 3, {0.4788,-0.8457,0.2358}},
        {{-0.006,-1.1008,-1.2077}, 3, {-0.0037,-0.6736,-0.7391}},
        {{-1.3236,-1.6466,0.7414}, 3, {-0.5911,-0.7354,0.3312}},
        {{-1.9842,0.0488,1.2505}, 3, {-0.8458,0.0208,0.5331}},
        {{0.887,-0.8325,1.6711}, 3, {0.4291,-0.4028,0.8085}},
        {{0.1702,-1.4313,-0.5066}, 3, {0.1114,-0.9368,-0.3316}},
        {{-0.2327,-0.2639,0.4711}, 3, {-0.3957,-0.4489,0.8012}},
        {{0.6016,0.4042,1.2209}, 3, {0.4237,0.2847,0.8599}},
        {{1.6346,-0.7231,-1.6382}, 3, {0.6742,-0.2982,-0.6757}},
        {{-1.5441,1.3147,-1.8124}, 3, {-0.5677,0.4834,-0.6664}},    
    };
    double result_data[3];
    gsl_vector_view test;
    gsl_vector_view result;
    int i;
    for (i=0; i<14; i++) {
        test = gsl_vector_view_array(test_cases[i].a, test_cases[i].len);
        result = gsl_vector_view_array(result_data, test_cases[i].len);
        gsl_vector_memcpy(&result.vector, &test.vector);
        normalise(&result.vector);
        snprintf(text,24,"Iteration: %d", i);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(test_cases[i].result, result_data, test_cases[i].len, text);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, gsl_blas_dnrm2(&result.vector));
    }
}
    
void test_sqrtm() {
    char text[80];
    struct test_field {
        double data[9];
        double result[9];
    };
    struct test_field test_cases[5] = {
        {{3.7713,0.3848,0.4159, 0.3848,3.4985,0.4927, 0.4159,0.4927,3.1691} , {1.9365,0.0975,0.1087, 0.0975,1.8632,0.1326, 0.1087,0.1326,1.7719}},
        {{3.0883,0.3447,0.783, 0.3447,3.5122,0.7672, 0.783,0.7672,3.2919} , {1.7419,0.0833,0.2169, 0.0833,1.861,0.2052, 0.2169,0.2052,1.7896}},
        {{3.9178,0.4284,0.4922, 0.4284,3.3733,0.5541, 0.4922,0.5541,3.6178} , {1.9725,0.108,0.1233, 0.108,1.8277,0.1454, 0.1233,0.1454,1.8925}},
        {{3.5131,0.7278,0.4601, 0.7278,3.5216,0.4996, 0.4601,0.4996,3.3007} , {1.8608,0.1913,0.1186, 0.1913,1.8623,0.1299, 0.1186,0.1299,1.8082}},
        {{3.114,0.7275,0.1229, 0.7275,3.5476,0.8381, 0.1229,0.8381,3.3517} , {1.7531,0.2,0.0217, 0.2,1.8591,0.2268, 0.0217,0.2268,1.8165}}
    };
    int i;
    double result_array[9];
    gsl_matrix_view result;
    gsl_matrix_view data;
    result = gsl_matrix_view_array(result_array, 3, 3);
    for (i = 0; i<5; i++) {
        data = gsl_matrix_view_array(test_cases[i].data, 3, 3);
        sqrtm(&data.matrix, &result.matrix);
        snprintf(text, 80, "Iteration: %d", i);
        TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(test_cases[i].result, result_array, 9, text);
    }
}

void test_fit_ellipsoid() {
    CEXCEPTION_T e;
    const char *file;
    const char *reason;
    int line;
    char text[80];
    CALIBRATION_DECLARE(result);
    GSL_VECTOR_DECLARE(v_result,3);
    int k;
    double magnitude;
    gsl_matrix_view cal1_m = gsl_matrix_view_array(mag_sample_data,16,3);
    #ifdef __MPLAB_DEBUGGER_SIMULATOR
    TEST_IGNORE_MESSAGE("This fails on simulator due to bugs in the sim :(");
    #endif
    Try {
        fit_ellipsoid(&cal1_m.matrix, &result);
    }
    Catch(e) {
        exception_get_details(&reason, &file, &line);
        printf("%s: %s: %s: %d", exception_get_string(e), reason, file, line);
    }
    ////print transformation code
    //printf("calibration transform:\n");
    //gsl_matrix_fprintf(stdout, &result.transform.matrix,"%f");
    //gsl_vector_fprintf(stdout, &result.offset.vector,"%f");
    for (k=0; k<cal1_m.matrix.size1; k++) {
        gsl_vector_view row = gsl_matrix_row(&cal1_m.matrix,k);
        apply_calibration(&row.vector, &result, &v_result);
        magnitude = gsl_blas_dnrm2(&v_result);
        ////print magnitudes
        //printf("mag: %f\n", fabs(1.0-magnitude));
        //snprintf(text, 80, "Data: %d", k);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.04, 1.0, magnitude, text);
    }
}

void run_all_calibration_on_data(const gsl_matrix *data, calibration *cal) {
    //calibrate and adjust planar shots...
    gsl_matrix_const_view spins = gsl_matrix_const_submatrix(data,8,0,8,3);
    fit_ellipsoid(data, cal);
}

void print_accuracy(const gsl_matrix *mag, const gsl_matrix *grav) {
    GSL_MATRIX_DECLARE(orientation, 8, 3);
    int i;
    for (i=0; i<8; i++) {
        gsl_vector_const_view mag_row = gsl_matrix_const_row(mag, i+8);
        gsl_vector_const_view grav_row = gsl_matrix_const_row(grav, i+8);
        gsl_vector_view orient_row = gsl_matrix_row(&orientation, i);
        maths_get_orientation_as_vector(&mag_row.vector,
                                        &grav_row.vector,
                                        &orient_row.vector);
        double x = gsl_vector_get(&orient_row.vector, 0);
        double y = gsl_vector_get(&orient_row.vector, 1);
        double z = gsl_vector_get(&orient_row.vector, 2);
        double compass = atan2(x, y) * DEGREES_PER_RADIAN;
        if (compass < 0) compass +=360;
        double extension =sqrt((x*x)+(y*y)); 
        printf("Orientation: %d\t%f\t%+f\n", 
                i, 
                compass, 
                atan2(z, extension) * DEGREES_PER_RADIAN);
    }
    printf("Total Cal:\n");
    for (i=0; i<3; i++) {
        gsl_vector_view column = gsl_matrix_column(&orientation, i);
        double error = gsl_stats_absdev(column.vector.data, column.vector.stride, 8);
        printf("Stdev: %d, %f\n", i, error * DEGREES_PER_RADIAN);
    }
    
}

void apply_calibration_to_matrix(const gsl_matrix *input, const calibration *cal, gsl_matrix *output) {
    size_t i;
    for (i=0; i< input->size1; ++i) {
        gsl_vector_const_view in_row = gsl_matrix_const_row(input, i);
        gsl_vector_view out_row = gsl_matrix_row(output, i);
        apply_calibration(&in_row.vector, cal, &out_row.vector);
    }
}


void test_all_calibration() {
    CALIBRATION_DECLARE(mag_cal);
    CALIBRATION_DECLARE(grav_cal);
    GSL_MATRIX_DECLARE(mag_results, 16, 3);
    GSL_MATRIX_DECLARE(grav_results, 16, 3);

    gsl_matrix_view mag_view = gsl_matrix_view_array(mag_sample_data,16,3);
    gsl_matrix_view grav_view = gsl_matrix_view_array(grav_sample_data,16,3);

    printf("Before calibration");
    print_accuracy(&mag_view.matrix, &grav_view.matrix);

    printf("Before syncing\n");
    run_all_calibration_on_data(&mag_view.matrix, &mag_cal);
    run_all_calibration_on_data(&grav_view.matrix, &grav_cal);
    apply_calibration_to_matrix(&mag_view.matrix, &mag_cal, &mag_results);
    apply_calibration_to_matrix(&grav_view.matrix, &grav_cal, &grav_results);
    print_accuracy(&mag_results, &grav_results);
}
