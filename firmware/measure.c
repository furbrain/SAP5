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

#define FEET_PER_METRE 3.281
#define DEGREES_PER_RADIAN 57.296
#define GRADS_PER_DEGREE 1.111111111



const char *cartesian_items[] = {"East:","North:","Vert:","Lg:    Ext:"};
const char *polar_items[] = {"Comp:","Clino:","Dist:","Lg:    Ext:"};

const char cartesian_format[] = " %+.2f ";
const char *polar_format[] = {" %03.1f "," %+02.1f "," %.2f "," %.2f "};


void get_readings(accum *orientation, accum *distance){
	int i,j;
	struct COOKED_SENSORS sensors;
	accum mags[3] = {0,0,0};
	accum accels[3] = {0,0,0};

	display_on(false);
	laser_off();
	delay_ms(20);
	for(i=0;i<8;++i) {
        wdt_clear();
		delay_ms(10);
		sensors_read_uncalibrated(&sensors);
		for (j=0;j<3;++j) {
			mags[j] += sensors.mag[j];
			accels[j] += sensors.accel[j];
		}
	}
	for(j=0;j<3;++j) {
		sensors.mag[j] = mags[j]/80;
		sensors.accel[j] = accels[j]/40;
	}
	sensors_uncalibrated_to_cooked(&sensors);
	sensors_get_orientation(&sensors,orientation);
	normalise(orientation);
    *distance = laser_read(LASER_MEDIUM,1000);
	laser_on();
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
	int32_t readings[15];
	char text[17];
	int i, count;
	struct RAW_SENSORS sensors;
	display_on(false);
	laser_off();
    sensors_read_raw(&sensors);
	for(i=0;i<3;++i) {
		count = 0;
		sensors_read_raw(&sensors);
		readings[i] = sensors.mag[i];
		readings[3+i] = sensors.accel[i];
        readings[6+i] = sensors.gyro[i];
		delay_ms(100);
        wdt_clear();
	}
	display_on(true);
	for(i=0;i<3;++i) {
		sprintf(text,"%5d%5d%5d",readings[i*3],readings[(i*3)+1],readings[(i*3)+2]);
		display_write_text(i*2,0,text,&small_font,false);
	}
	laser_on();
    wdt_clear();
	delay_ms(800);
    wdt_clear();
	return;
}

void measure(int32_t a) {
	int item = 0;
	bool readings_available = false;
	accum items[4];
	accum orientation[4];
	accum extension,distance;
	int i, cycle;
	char text[17];
	char format[17];
	char degree_sign;
	char length_sign;
	distance = 10.0;
	length_sign = (config.length_units==IMPERIAL)?'\'':'m';
	degree_sign = (config.display_style==GRAD)?'g':'`';
	cycle = 0;
	//test stuff here...
// 	test();
// 	return;
	//end test
    laser_on();
	display_clear_screen();
	display_write_text(4,0,"*----",&large_font,false);
	while (true) {
		cycle++;
		if (readings_available) {
			for (i=0; i<4; i++) {
				if (config.display_style==CARTESIAN) {
					sprintf(format,cartesian_format,(double)items[i]);
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
				if (i==3) {
					display_write_text(6,26,"123",&small_font,false);
				}
				display_write_text(i*2,127,format,&small_font,true);
			}
		}
		switch(get_action()) {
			case FLIP_UP:
			case FLIP_DOWN:
				break;
			case NONE:
				if (cycle<30) break;
				cycle=0;
			case SINGLE_CLICK:
			case LONG_CLICK:
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
				/* take measurement */
				readings_available = true;
				break;
			case DOUBLE_CLICK:
                laser_off();
				return;
				break;
		}
        wdt_clear();
		delay_ms(50);
        
	}
}
