#include "unity.h"
#include "measure.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include "gsl_static.h"
#include "mock_display.h"
#include "mock_laser.h"
#include "mock_sensors.h"
#include "mock_utils.h"
#include "menu.h"
#include "mock_maths.h"
#include "mock_interface.h"
#include "mock_memory.h"
#include "survey.h"
#include "leg.h"
#include "config.h"
#include "exception.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_get_extension(void) {
    struct test_field {
        double deltas[3];
        double result;
    };
    struct test_field test_cases[] = {
        {{0,0,0},0},
        {{1,0,0},1},
        {{0,1,0},1},
        {{1,0,1},1},
        {{0,-1,1},1},
        {{0.3, 0.4, 0.0}, 0.5},
        {{0.3, -0.4, 0.2}, 0.5}
    };
    
    int i;
    double result;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        result = get_extension(&orientation.vector);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].result, result);
    }
}

void test_get_distance(void) {
    struct test_field {
        double deltas[3];
        double result;
    };
    struct test_field test_cases[] = {
        {{0,0,0},0},
        {{1,0,0},1},
        {{0,1,0},1},
        {{3,0,4},5},
        {{0,-3,4},5},
        {{0.3, 0.4, 0.0}, 0.5},
        {{0.3, -0.4, 1.2}, 1.3}
    };
    
    int i;
    double result;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        result = get_distance(&orientation.vector);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].result, result);
    }
}

void test_calculate_bearings(void) {
    struct test_field {
        double deltas[3];
        double compass;
        double inclination;
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, 0, 0},
        {{1, 0, 0}, 90, 0},
        {{0, 1, 0}, 0, 0},
        {{3, 0, 4}, 90, 53.130},
        {{0, -3, 4}, 180, 53.130},
        {{0.3, 0.4, 0.0}, 36.870, 0},
        {{0.3, -0.4, 1.2}, 143.130, 67.38}
    };
    int i;
    double compass, inclination;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        calculate_bearings(&orientation.vector, &compass, &inclination);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].compass, compass);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].inclination, inclination);
    }
}

void test_add_polar_entries_to_menu(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][12];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0'", "+0.0'", "Dst 0.00", "Ext 0.00"},
        {{1, 0, 0}, "090.0'", "+0.0'", "Dst 1.00", "Ext 1.00"},
        {{0, 1, 0}, "000.0'", "+0.0'", "Dst 1.00", "Ext 1.00"},
        {{3, 0, 4}, "090.0'", "+53.1'","Dst 5.00", "Ext 3.00"},
        {{0, -3, 4}, "180.0'", "+53.1'","Dst 5.00", "Ext 3.00"},
        {{0.3, 0.4, 0.0}, "036.9'", "+0.0'", "Dst 0.50", "Ext 0.50"},
        {{0.3, -0.4, 1.2}, "143.1'", "+67.4'", "Dst 1.30", "Ext 0.50"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        menu_clear(&test_menu);
        add_polar_entries_to_menu(&orientation.vector, &test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}

void test_add_cartesian_entries_to_menu(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][12];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "E: +0.00", "N: +0.00", "V: +0.00", "Ext 0.00"},
        {{1, 0, 0}, "E: +1.00", "N: +0.00", "V: +0.00", "Ext 1.00"},
        {{0, 1, 0}, "E: +0.00", "N: +1.00", "V: +0.00", "Ext 1.00"},
        {{3, 0, 4}, "E: +3.00", "N: +0.00", "V: +4.00", "Ext 3.00"},
        {{0, -3, 4}, "E: +0.00", "N: -3.00", "V: +4.00", "Ext 3.00"},
        {{0.3, 0.4, 0.0}, "E: +0.30", "N: +0.40", "V: +0.00", "Ext 0.50"},
        {{0.3, -0.4, 1.2}, "E: +0.30", "N: -0.40", "V: +1.20", "Ext 0.50"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        menu_clear(&test_menu);
        add_cartesian_entries_to_menu(&orientation.vector, &test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}