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
#include "survey.h"

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
        { {0.6737,0.1964,0.3544} , {-0.051341790263912723, -0.74861071358699505} },
        { {0.8134,0.2478,0.4586} , {-0.035593591306653682, -0.92278282148962165} },
        { {0.8773,0.3917,0.3261} , {-0.11235045375781333, -0.89108587892891877} },
        { {0.0901,0.8342,0.0962} , {0.34133319039960425, -0.13030209039896268} },
        { {0.6475,0.5495,0.8497} , {0.45913654500365642, -1.0343935191750973} },
        { {0.2653,0.4409,0.8809} , {0.65760305863875168, -0.75330579800963537} },
        { {0.7364,0.7126,0.0357} , {-0.11813308458308652, -0.60068514360971081} },
        { {0.0197,0.3278,0.563} , {0.52357307700114608, -0.36372692579436477} },
        { {0.7457,0.7296,0.9902} , {0.5746297593241827, -1.198403851389874} },
        { {0.9635,0.5502,0.9883} , {0.37959826350168346, -1.3683293076476934} },
        { {0.5093,0.3498,0.9313} , {0.5197654308753632, -0.97618252276124862} },
        { {0.2042,0.4012,0.4722} , {0.38099952941083137, -0.45249493398715779} },
        { {0.1378,0.7252,0.5857} , {0.62656295436851206, -0.4705778952392039} },
        { {0.1565,0.7861,0.1653} , {0.33489285207753949, -0.22525693361599494} },
        { {0.7443,0.2593,0.7522} , {0.22092064580275, -1.0501504780824995} },
        { {0.1289,0.0254,0.1766} , {0.064256262529628516, -0.21052164144011892} },
        { {0.9675,0.2823,0.8263} , {0.15626662459395946, -1.2712841853114329} },
        { {0.3722,0.469,0.1884} , {0.10585160663829837, -0.40895411431210332} },
        { {0.1527,0.0016,0.4586} , {0.24580757488141125, -0.40367217837533637} },
        { {0.0388,0.7333,0.6793} , {0.75396056048536297, -0.45064410843265151} },        
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
    TEST_ASSERT_EQUAL(32, transform.scale);
    TEST_ASSERT_EQUAL(0, transform.offset[1]);
    TEST_ASSERT_EQUAL(-0.5, transform.offset[0]);
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
        {12,24},    //{1,0,0}, -> {0,-1} -> {1,0}
        {10,22},    //{0,1,0}, -> {1,0} -> {0,-1}
        {10,24},    //{0,0,1}, ->  {0,0} -> {0,0}
        {8,24},    //{-1,0,0} -> {0,1} -> {-1, 0}
    };
    int i;
    for (i=0; i<5; ++i) {
        translate_station(&cave.stations[i], &transform, &x, &y);
        TEST_ASSERT_EQUAL(test_cases[i].x + (DISPLAY_WIDTH/2), x);
        TEST_ASSERT_EQUAL(test_cases[i].y + (DISPLAY_HEIGHT/2), y);
    }
}

void test_display_cave(void) {
    GSL_MATRIX_DECLARE(orientation, 3, 3);
    gsl_matrix_set_identity(&orientation);
                                            // origin = 64,32
    display_clear_screen_Expect(false);     //scl=32 ofs = {0.5, 0.0}
    display_draw_line_Expect(80,32,80,0); //{0,0,0} -> {0.5,0}
    display_draw_line_Expect(80,0,48,32);  //{1,0,0} -> {0.5,-1}
    display_draw_line_Expect(48,32,80,32);   //{0,1,0} -> {-0.5,0}
    display_draw_line_Expect(80,32,80,64);   //{0,0,1} -> {0.5,0}
    display_show_buffer_Expect();           //{-1,0,0} ->{0.5,1}   

    display_cave(&cave, &orientation);
}