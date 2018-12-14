#include "config.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdfix.h>
#include "measure.h"
#include "interface.h"
#include "display.h"
#include "sensors.h"
#include "maths.h"
#include "laser.h"
#include "menu.h"

#define FEET_PER_METRE 3.281
#define DEGREES_PER_RADIAN 57.296
#define GRADS_PER_DEGREE 1.111111111
#define NUM_SENSOR_READINGS 10


const char *cartesian_items[] = {"East:","North:","Vert:","Ext:"};
const char *polar_items[] = {"Comp:","Clino:","Dist:","Ext:"};

const char cartesian_small_format[] = " %+.2f ";
const char *cartesian_big_format[] = {"E: %+.2f", "N: %+.2f","V: %+.2f","Ext%+.2f"};
const char *polar_format[] = {" %03.1f "," %+02.1f "," %.2f "," %.2f "};

double deltas[4];

static bool measure_exit;

DECLARE_EMPTY_MENU(leg_menu, 20);

void get_readings(gsl_vector *orientation){
    GSL_VECTOR_DECLARE(magnetism, 3);
    GSL_VECTOR_DECLARE(acceleration, 3);
	struct COOKED_SENSORS sensors;
	double distance;
    gsl_vector_view sensors_magnetism = gsl_vector_view_array(sensors.mag, 3);    
    gsl_vector_view sensors_acceleration = gsl_vector_view_array(sensors.accel, 3);    
	int i;

	display_on(false);
	laser_on(true);
	delay_ms_safe(20);
	gsl_vector_set_zero(&magnetism);
	gsl_vector_set_zero(&acceleration);
	for(i=0; i < NUM_SENSOR_READINGS; ++i) {
		delay_ms_safe(10);
        sensors_read_cooked(&sensors);
        gsl_vector_add(&magnetism, &sensors_magnetism.vector);
        gsl_vector_add(&acceleration, &sensors_acceleration.vector);
    }
    gsl_vector_scale(&magnetism, 1.0/NUM_SENSOR_READINGS);
    gsl_vector_scale(&acceleration, 1.0/NUM_SENSOR_READINGS);
    distance = laser_read(LASER_MEDIUM, 1000);
    maths_get_orientation(&magnetism, &acceleration, orientation);
    display_on(true);
    laser_on(false);
}

void measure_get_readings(gsl_vector *orientation) {
    while (true) {
 		switch(get_action()) {
			case SINGLE_CLICK:
			case LONG_CLICK:
				/* take measurement */
				get_readings(orientation);
				return;
				break;
			case DOUBLE_CLICK:
			    measure_exit = true;
				return;
				break;
	        default:
   }
}

double get_extension(double *o, double distance) {
	//remember orientations are normalised...
	return sqrt(o[0]*o[0]+o[1]*o[1])*distance;
}
/* set items to compass,clino,distance and extension, respectively */
void calculate_bearings(double *orientation, double *items, double distance){
	items[0] = atan2(orientation[0],orientation[1])*DEGREES_PER_RADIAN;
	if (items[0]<0) items[0]+=360;
	items[1] = atan2(orientation[2],get_extension(orientation,1))*DEGREES_PER_RADIAN;
	if (config.display_style==GRAD) {
		items[0] *= GRADS_PER_DEGREE;
		items[1] *= GRADS_PER_DEGREE;
	}
	items[2] = distance;
	items[3] = get_extension(orientation,distance);
	if (config.length_units==IMPERIAL) {
		items[2] *= FEET_PER_METRE;
		items[3] *= FEET_PER_METRE;
	}
}

/* set items to easting, northing, vertical offset and extension, respectively */
void calculate_deltas(double *orientation, double *items, double distance){
	int i;
	for(i=0;i<3;++i) {
		items[i] = orientation[i]*distance;
	}
	items[3] = get_extension(orientation,distance);
	if (config.length_units==IMPERIAL) {
		for (i=0;i<4;++i) {
			items[i] *= FEET_PER_METRE;
		}
	}
}

void test() {
    struct COOKED_SENSORS sensors;
    char text[20];
    int i;
    do {
        wdt_clear();
        sensors_read_cooked(&sensors);
        for (i=0; i<3; i++) {
            sprintf(text,"%6.2f",(double)sensors.mag[i]);
            display_write_text(i*2, 0, text, &small_font, false);
        }
        wdt_clear();
        delay_ms(200);
        wdt_clear();
    } while (get_action()==NONE);
}

void store_leg(int32_t info) {
    ///FIXME
}

bool measurement_menu(double *items) {
    int index = 1;
    int i;
    char text[20];
/*    for (i=0; i<4; i++) {
        if (config.display_style==CARTESIAN) {
            
            snprintf(text,20,cartesian_big_format[i],items[i]);
            menu_set_entry(leg_menu+index, i, text, INFO, NULL, 0);
            index++;
        }
    }

    //add store option
    menu_set_entry(leg_menu+index, index-1, "Store", 10, NULL, 0);
    index++;

    //add discard option
    menu_set_entry(leg_menu+index,index-1,"Discard",FUNCTION, NULL, 0);
    index++;
    
    //add main menu option
    menu_set_entry(leg_menu+index, index-1, "Main Menu", BACK, NULL, 0);
    index++;
    
    //end of top level menu
    menu_set_entry(leg_menu+index, index-1, "", 0, NULL, 0);
    
    index =10;
    menu_set_entry(leg_menu+index, index, "1->2", FUNCTION, store_leg, 12);
    index++;
    menu_set_entry(leg_menu+index, index, "1-> - ", FUNCTION, store_leg, 10);
    index++;
    menu_set_entry(leg_menu+index, index, "2->1", FUNCTION, store_leg, 21);
    index++;
    menu_set_entry(leg_menu+index, index, "2-> - ", FUNCTION, store_leg, 20);
    index++;
    menu_set_entry(leg_menu+index, index, "CUSTOM", FUNCTION, store_leg, 00);
    index++;
    menu_set_entry(leg_menu+index, index, "CUSTOM", BACK, NULL, 00);
    index++;
    menu_set_entry(leg_menu+index, -1, "", BACK, NULL, 00);
    return show_menu(leg_menu,1,false);
 */
}

void new_measure(int32_t a) {
    GSL_VECTOR_DECLARE(orientation, 3);
    measure_exit = false;
    laser_on(true);
    while (true) {
        measure_get_reading(orientation);
        if (measure_exit) break;
        measure_show_reading(orientation);
        if (measure_exit) break;
    }        
    laser_on(false);
    display_on(true);
}



void measure(int32_t a) {
	int item = 0;
	bool readings_available = false;
	double items[4];
	double orientation[4];
	double extension,distance;
	int i;
	char text[17];
	char format[17];
	char degree_sign;
	char length_sign;
//    test();    return;
	distance = 10.0;
	length_sign = (config.length_units==IMPERIAL)?'\'':'m';
	degree_sign = (config.display_style==GRAD)?'g':'`';
	//test stuff here...
// 	test();
// 	return;
	//end test
    laser_on(true);
	display_clear_screen();
	display_write_text(4,0,"*----",&large_font,false);
	while (true) {
		if (readings_available) {
			for (i=0; i<4; i++) {
				if (config.display_style==CARTESIAN) {
					sprintf(format,cartesian_small_format,(double)items[i]);
					display_write_text(i*2,0,cartesian_items[i],&small_font,false);
				} else {
					sprintf(format,polar_format[i],(double)items[i]);
					display_write_text(i*2,0,polar_items[i],&small_font,false);
				}
				if ((i<2) && (config.display_style!=CARTESIAN)) {
					format[strlen(format)-1] = degree_sign;
				} else {
					format[strlen(format)-1] = length_sign;
				}
				display_write_text(i*2,127,format,&small_font,true);
			}
		}
		switch(get_action()) {
			case FLIP_UP:
			case FLIP_DOWN:
                if (readings_available) {
                    if (measurement_menu(items)) {
                        
                    }
                }
				break;
			case SINGLE_CLICK:
			case LONG_CLICK:
				/* take measurement */
				get_readings(orientation,&distance);
				switch (config.display_style) {
					case GRAD:
					case POLAR:
						calculate_bearings(orientation,
										   items,
										   distance);
						break;
					case CARTESIAN:
						calculate_deltas(orientation,
										 items,
										 distance);
						break;
                        
				}
                calculate_deltas(orientation, deltas, distance);
				readings_available = true;
				break;
			case DOUBLE_CLICK:
                laser_on(false);
				return;
				break;
		}
        wdt_clear();
		delay_ms(50);
        
	}
}
