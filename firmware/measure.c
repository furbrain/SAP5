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



const char *cartesian_items[] = {"East:","North:","Vert:","Ext:"};
const char *polar_items[] = {"Comp:","Clino:","Dist:","Ext:"};

const char cartesian_small_format[] = " %+.2f ";
const char *cartesian_big_format[] = {"E: %+.2f", "N: %+.2f","V: %+.2f","Ext%+.2f"};
const char *polar_format[] = {" %03.1f "," %+02.1f "," %.2f "," %.2f "};

accum deltas[4];

DECLARE_EMPTY_MENU(leg_menu, 20);

void get_readings(accum *orientation, accum *distance){
	int i,j;
	struct COOKED_SENSORS sensors;
	accum mags[3] = {0,0,0};
	accum accels[3] = {0,0,0};

	display_on(false);
	laser_on(false);
	delay_ms(20);
    sensors_read_cooked(&sensors);
//	for(i=0;i<8;++i) {
//        wdt_clear();
//		delay_ms(10);
//		sensors_read_uncalibrated(&sensors);
//		for (j=0;j<3;++j) {
//			mags[j] += sensors.mag[j];
//			accels[j] += sensors.accel[j];
//		}
//	}
//	for(j=0;j<3;++j) {
//		sensors.mag[j] = mags[j]/80;
//		sensors.accel[j] = accels[j]/40;
//	}
	sensors_uncalibrated_to_cooked(&sensors);
	sensors_get_orientation(&sensors,orientation);
	normalise(orientation,3);
    *distance = laser_read(LASER_MEDIUM,1000);
	laser_on(true);
	display_on(true);
}


accum get_extension(accum *o, accum distance) {
	//remember orientations are normalised...
	return sqrt(o[0]*o[0]+o[1]*o[1])*distance;
}
/* set items to compass,clino,distance and extension, respectively */
void calculate_bearings(accum *orientation, accum *items, accum distance){
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
void calculate_deltas(accum *orientation, accum *items, accum distance){
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

bool measurement_menu(accum *items) {
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

void measure(int32_t a) {
	int item = 0;
	bool readings_available = false;
	accum items[4];
	accum orientation[4];
	accum extension,distance;
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
