#include "unity.h"
#include "measure.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include "gsl_static.h"
#include "mock_display.h"
#include "mock_laser.h"
#include "mock_sensors.h"
#include "mock_ui.h"
#include "mock_utils.h"
#include "menu.h"
#include "font.h"
#include "mock_maths.h"
#include "mock_interface.h"
#include "mock_memory.h"
#include "mock_beep.h"
#include "mock_input.h"
#include "mock_display.h"
#include "images.h"
#include "survey.h"
#include "leg.h"
#include "config.h"
#include "exception.h"

DECLARE_MENU(main_menu, {
    {"30s", Info, {NULL}, 0},
    {"Back", Back, {NULL}, 0}
});

display_buf_t display_buffer;

void setUp(void) {
    config.length_units=METRIC;
    config.display_style=POLAR;
    config.compact=false;
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
        measure_calculate_bearings(&orientation.vector, &compass, &inclination);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].compass, compass);
        TEST_ASSERT_EQUAL_DOUBLE(test_cases[i].inclination, inclination);
    }
}



void test_add_polar_entries_to_menu(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0`", "+0.0`", "Dist  0.00m", "Ext  0.00m"},
        {{1, 0, 0}, "090.0`", "+0.0`", "Dist  1.00m", "Ext  1.00m"},
        {{0, 1, 0}, "000.0`", "+0.0`", "Dist  1.00m", "Ext  1.00m"},
        {{3, 0, 4}, "090.0`", "+53.1`","Dist  5.00m", "Ext  3.00m"},
        {{0, -3, 4}, "180.0`", "+53.1`","Dist  5.00m", "Ext  3.00m"},
        {{0.3, 0.4, 0.0}, "036.9`", "+0.0`", "Dist  0.50m", "Ext  0.50m"},
        {{0.3, -0.4, 1.2}, "143.1`", "+67.4`", "Dist  1.30m", "Ext  0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        menu_clear(&test_menu);
        add_polar_entries_to_menu(&test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}

void test_polar_strings(void) {
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0`", "+0.0`", "0.00m", "0.00m"},
        {{1, 0, 0}, "090.0`", "+0.0`", "1.00m", "1.00m"},
        {{0, 1, 0}, "000.0`", "+0.0`", "1.00m", "1.00m"},
        {{3, 0, 4}, "090.0`", "+53.1`","5.00m", "3.00m"},
        {{0, -3, 4}, "180.0`", "+53.1`","5.00m", "3.00m"},
        {{0.3, 0.4, 0.0}, "036.9`", "+0.0`", "0.50m", "0.50m"},
        {{0.3, -0.4, 1.2}, "143.1`", "+67.4`", "1.30m", "0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        fill_polar_strings();
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], texts.raw_text[j]);
        }
    }  
}

void test_add_polar_entries_to_menu_imperial(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0`", "+0.0`", "Dist  0.00'", "Ext  0.00'"},
        {{1, 0, 0}, "090.0`", "+0.0`", "Dist  3.28'", "Ext  3.28'"},
        {{0, 1, 0}, "000.0`", "+0.0`", "Dist  3.28'", "Ext  3.28'"},
        {{3, 0, 4}, "090.0`", "+53.1`", "Dist  16.41'", "Ext  9.84'"},
        {{0, -3, 4}, "180.0`", "+53.1`", "Dist  16.41'", "Ext  9.84'"},
        {{0.3, 0.4, 0.0}, "036.9`", "+0.0`", "Dist  1.64'", "Ext  1.64'"},
        {{0.3, -0.4, 1.2}, "143.1`", "+67.4`", "Dist  4.27'", "Ext  1.64'"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.length_units=IMPERIAL;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        menu_clear(&test_menu);
        add_polar_entries_to_menu(&test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}

void test_fill_polar_strings_imperial(void) {
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0`", "+0.0`", "0.00'", "0.00'"},
        {{1, 0, 0}, "090.0`", "+0.0`", "3.28'", "3.28'"},
        {{0, 1, 0}, "000.0`", "+0.0`", "3.28'", "3.28'"},
        {{3, 0, 4}, "090.0`", "+53.1`", "16.41'", "9.84'"},
        {{0, -3, 4}, "180.0`", "+53.1`", "16.41'", "9.84'"},
        {{0.3, 0.4, 0.0}, "036.9`", "+0.0`", "1.64'", "1.64'"},
        {{0.3, -0.4, 1.2}, "143.1`", "+67.4`", "4.27'", "1.64'"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.length_units=IMPERIAL;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        fill_polar_strings();
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], texts.raw_text[j]);
        }
    }  
}


void test_add_gradian_entries_to_menu(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0g", "+0.0g", "Dist  0.00m", "Ext  0.00m"},
        {{1, 0, 0}, "100.0g", "+0.0g", "Dist  1.00m", "Ext  1.00m"},
        {{0, 1, 0}, "000.0g", "+0.0g", "Dist  1.00m", "Ext  1.00m"},
        {{3, 0, 4}, "100.0g", "+59.0g","Dist  5.00m", "Ext  3.00m"},
        {{0, -3, 4}, "200.0g", "+59.0g","Dist  5.00m", "Ext  3.00m"},
        {{0.3, 0.4, 0.0}, "041.0g", "+0.0g", "Dist  0.50m", "Ext  0.50m"},
        {{0.3, -0.4, 1.2}, "159.0g", "+74.9g", "Dist  1.30m", "Ext  0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.display_style=GRAD;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);

        menu_clear(&test_menu);
        add_polar_entries_to_menu(&test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}

void test_fill_gradian_strings(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "000.0g", "+0.0g", "0.00m", "0.00m"},
        {{1, 0, 0}, "100.0g", "+0.0g", "1.00m", "1.00m"},
        {{0, 1, 0}, "000.0g", "+0.0g", "1.00m", "1.00m"},
        {{3, 0, 4}, "100.0g", "+59.0g","5.00m", "3.00m"},
        {{0, -3, 4}, "200.0g", "+59.0g","5.00m", "3.00m"},
        {{0.3, 0.4, 0.0}, "041.0g", "+0.0g", "0.50m", "0.50m"},
        {{0.3, -0.4, 1.2}, "159.0g", "+74.9g", "1.30m", "0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.display_style=GRAD;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        fill_polar_strings();
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], texts.raw_text[j]);
        }
    }  
}



void test_add_cartesian_entries_to_menu(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "E: +0.00m", "N: +0.00m", "V: +0.00m", "Ext  0.00m"},
        {{1, 0, 0}, "E: +1.00m", "N: +0.00m", "V: +0.00m", "Ext  1.00m"},
        {{0, 1, 0}, "E: +0.00m", "N: +1.00m", "V: +0.00m", "Ext  1.00m"},
        {{3, 0, 4}, "E: +3.00m", "N: +0.00m", "V: +4.00m", "Ext  3.00m"},
        {{0, -3, 4}, "E: +0.00m", "N: -3.00m", "V: +4.00m", "Ext  3.00m"},
        {{0.3, 0.4, 0.0}, "E: +0.30m", "N: +0.40m", "V: +0.00m", "Ext  0.50m"},
        {{0.3, -0.4, 1.2}, "E: +0.30m", "N: -0.40m", "V: +1.20m", "Ext  0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);        
        menu_clear(&test_menu);
        add_cartesian_entries_to_menu(&test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}

void test_fill_cartesian_strings(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "+0.00m", "+0.00m", "+0.00m", "0.00m"},
        {{1, 0, 0}, "+1.00m", "+0.00m", "+0.00m", "1.00m"},
        {{0, 1, 0}, "+0.00m", "+1.00m", "+0.00m", "1.00m"},
        {{3, 0, 4}, "+3.00m", "+0.00m", "+4.00m", "3.00m"},
        {{0, -3, 4}, "+0.00m", "-3.00m", "+4.00m", "3.00m"},
        {{0.3, 0.4, 0.0}, "+0.30m", "+0.40m", "+0.00m", "0.50m"},
        {{0.3, -0.4, 1.2}, "+0.30m", "-0.40m", "+1.20m", "0.50m"}
    };
    int i, j;
    gsl_vector_view orientation;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        fill_cartesian_strings();
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], texts.raw_text[j]);
        }
    }  
}


void test_add_cartesian_entries_to_menu_imperial(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "E: +0.00'", "N: +0.00'", "V: +0.00'", "Ext  0.00'"},
        {{1, 0, 0}, "E: +3.28'", "N: +0.00'", "V: +0.00'", "Ext  3.28'"},
        {{0, 1, 0}, "E: +0.00'", "N: +3.28'", "V: +0.00'", "Ext  3.28'"},
        {{3, 0, 4}, "E: +9.84'", "N: +0.00'", "V: +13.12'", "Ext  9.84'"},
        {{0, -3, 4}, "E: +0.00'", "N: -9.84'", "V: +13.12'", "Ext  9.84'"},
        {{0.3, 0.4, 0.0}, "E: +0.98'", "N: +1.31'", "V: +0.00'", "Ext  1.64'"},
        {{0.3, -0.4, 1.2}, "E: +0.98'", "N: -1.31'", "V: +3.94'", "Ext  1.64'"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.length_units=IMPERIAL;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        menu_clear(&test_menu);
        add_cartesian_entries_to_menu(&test_menu);
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], menu_get_text(&test_menu));
            menu_next(&test_menu);
        }
    }  
}
    
void test_fill_cartesian_imperial(void) {
    DECLARE_EMPTY_MENU(test_menu, 4);
    struct test_field {
        double deltas[3];
        char texts[4][15];        
    };
    struct test_field test_cases[] = {
        {{0, 0, 0}, "+0.00'", "+0.00'", "+0.00'", "0.00'"},
        {{1, 0, 0}, "+3.28'", "+0.00'", "+0.00'", "3.28'"},
        {{0, 1, 0}, "+0.00'", "+3.28'", "+0.00'", "3.28'"},
        {{3, 0, 4}, "+9.84'", "+0.00'", "+13.12'", "9.84'"},
        {{0, -3, 4}, "+0.00'", "-9.84'", "+13.12'", "9.84'"},
        {{0.3, 0.4, 0.0}, "+0.98'", "+1.31'", "+0.00'", "1.64'"},
        {{0.3, -0.4, 1.2}, "+0.98'", "-1.31'", "+3.94'", "1.64'"}
    };
    int i, j;
    gsl_vector_view orientation;
    config.length_units=IMPERIAL;
    for (i=0; i<7; i++) {
        orientation = gsl_vector_view_array(test_cases[i].deltas, 3);
        sensors_get_last_reading_ExpectAndReturn(&orientation.vector);
        fill_cartesian_strings();
        for (j=0; j<4; j++) {
            TEST_ASSERT_EQUAL_STRING(test_cases[i].texts[j], texts.raw_text[j]);
        }
    }  
}
    
