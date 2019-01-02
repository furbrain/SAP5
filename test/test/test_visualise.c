#include "unity.h"
#include "visualise.h"
#include "model.h"
#include "leg.h"
#include "exception.h"
#include "gsl_static.h"
#include "mock_memory.h"
#include "mock_display.h"
#include "mock_interface.h"
#include "mock_sensors.h"
#include "mock_maths.h"
#include "mock_utils.h"

double station_array[][3] = {
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {0,0,1},
    {-1,0,0}
};

const int station_array_size = sizeof(station_array)/ sizeof(station_array[0]);
struct MODEL_CAVE cave;
void setUp(void)
{
    int i;
    reset_lists();
    for (i=0; i< station_array_size; ++i) {
        add_station(i, station_array[i]);
        if (i!=0) {
            add_leg(find_station(i-1), find_station(i));
        }
    }
    cave.stations = model_stations;
    cave.station_count = station_count;
    cave.legs = model_legs;
    cave.leg_count = model_leg_count;
}

void tearDown(void)
{
}

void test_convert_to_device(void)
{
    double orientation_data[] = {0.7858, 0.0, 0.6185, 0.5702, -0.3872, -0.7245, 0.2395,0.922,-0.3043};
    gsl_matrix_view  orientation = gsl_matrix_view_array(orientation_data, 3, 3);
    struct test_field {
        double xyz[3];
        double xy[2];
    };
    struct test_field test_cases[20] = {
        { {0.6737,0.1964,0.3544} , {0.7486,0.0513} },
        { {0.8134,0.2478,0.4586} , {0.9228,0.0356} },
        { {0.8773,0.3917,0.3261} , {0.8911,0.1124} },
        { {0.0901,0.8342,0.0962} , {0.1303,-0.3413} },
        { {0.6475,0.5495,0.8497} , {1.0344,-0.4591} },
        { {0.2653,0.4409,0.8809} , {0.7533,-0.6576} },
        { {0.7364,0.7126,0.0357} , {0.6007,0.1181} },
        { {0.0197,0.3278,0.563} , {0.3637,-0.5236} },
        { {0.7457,0.7296,0.9902} , {1.1984,-0.5746} },
        { {0.9635,0.5502,0.9883} , {1.3683,-0.3796} },
        { {0.5093,0.3498,0.9313} , {0.9762,-0.5198} },
        { {0.2042,0.4012,0.4722} , {0.4525,-0.381} },
        { {0.1378,0.7252,0.5857} , {0.4706,-0.6266} },
        { {0.1565,0.7861,0.1653} , {0.2253,-0.3349} },
        { {0.7443,0.2593,0.7522} , {1.0502,-0.2209} },
        { {0.1289,0.0254,0.1766} , {0.2105,-0.0643} },
        { {0.9675,0.2823,0.8263} , {1.2713,-0.1563} },
        { {0.3722,0.469,0.1884} , {0.409,-0.1059} },
        { {0.1527,0.0016,0.4586} , {0.4037,-0.2458} },
        { {0.0388,0.7333,0.6793} , {0.4506,-0.754} },
    };
    double x, y;
    int i;
    for (i=0; i< 20; ++i) {
        convert_to_device(&orientation.matrix, test_cases[i].xyz, &x, &y);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].xy[0], x);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].xy[1], y);
    }
}

void test_visualise_get_offset_and_scale(void) {
    GSL_MATRIX_DECLARE(tf_matrix, 3, 3);
    struct VISUALISE_TRANSFORMATION transform = {
        &tf_matrix,
        {0, 0},
        1.0
    };
    //with no rotation (ie device is pointing north with display up)
    gsl_matrix_set_identity(&tf_matrix);
    TEST_ASSERT_EQUAL(5, cave.station_count);
    TEST_ASSERT_EQUAL(4, cave.leg_count);
    get_offset_and_scale(&cave, &transform);
    TEST_ASSERT_EQUAL(64, transform.scale);
    TEST_ASSERT_EQUAL(0, transform.offset[0]);
    TEST_ASSERT_EQUAL(-0.5, transform.offset[1]);
}

void test_translate_station(void) {
    GSL_MATRIX_DECLARE(tf_matrix, 3, 3);
    struct VISUALISE_TRANSFORMATION transform = {
        &tf_matrix,
        {5, 12},
        2.0
    };
    int x,y;
    gsl_matrix_set_zero(&tf_matrix);
    //rotate everything 90 degrees
    gsl_matrix_set(&tf_matrix, 2, 2, 1.0);
    gsl_matrix_set(&tf_matrix, 0, 1, 1.0);
    gsl_matrix_set(&tf_matrix, 1, 0, -1.0);
    struct test_field {
        int x;
        int y;
    };
    struct test_field test_cases[] = {
        {10,24},    //{0,0,0},
        {10,22},    //{1,0,0}, -> {0,-1}
        {12,24},    //{0,1,0}, -> {1,0}
        {10,24},    //{0,0,1}, ->  {0,0}
        {10,26},    //{-1,0,0} -> {0,1}
    };
    int i;
    for (i=0; i<5; ++i) {
        translate_station(&cave.stations[i], &transform, &x, &y);
        TEST_ASSERT_EQUAL(test_cases[i].x + (DISPLAY_WIDTH/2), x);
        TEST_ASSERT_EQUAL((DISPLAY_HEIGHT/2) - test_cases[i].y, y);
    }
}

void test_display_cave(void) {
    GSL_MATRIX_DECLARE(orientation, 3, 3);
    gsl_matrix_set_identity(&orientation);

    display_clear_screen_Expect(false);
    display_draw_line_Expect(64,64,128,64);
    display_draw_line_Expect(128,64,64,0);
    display_draw_line_Expect(64,0,64,64);
    display_draw_line_Expect(64,64,0,64);
    display_show_buffer_Expect();

    display_cave(&cave, &orientation);
}