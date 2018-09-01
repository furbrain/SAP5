#include "config.h"
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "maths.h"

#define MIN(a,b) ((a)<(b))?(a):(b)
#define MAX(a,b) ((a)>(b))?(a):(b)
int get_greatest_axis(struct RAW_SENSORS *raw) {
    int max_abs = 0;
    int axis = 0;
    int i;
    for (i=0; i<3; i++) {
        if (abs(raw->accel[i])> max_abs) {
            if (raw->accel[i]) {
                axis = i;
            } else {
                axis = i+3;
            }
            max_abs = abs(raw->accel[i]);
        }
    }   
    return axis;
}

const int compass_axis_map[] = {1,0,5,4,3,2};

int get_compass_axis(int axis) {
    return compass_axis_map[axis];
}

void set_axes() {
    int i;
    struct RAW_SENSORS raw;
    int accel_axes[3];
    int compass_axes[3];
    char text[18];
    display_clear_screen();
    display_write_multiline(0, "Please place the\ndisplay flat on\na level surface", &small_font);
    for (i=0; i< 6; ++i) {
        wdt_clear();
        delay_ms(500);
    }
    wdt_clear();
    sensors_read_raw(&raw);
    accel_axes[2] = get_greatest_axis(&raw);
    compass_axes[2] = get_compass_axis(accel_axes[2]);
    wdt_clear();
    display_clear_screen();
    display_write_multiline(1, "Please point the\nlaser upwards", &small_font);
    for (i=0; i< 6; ++i) {
        wdt_clear();
        delay_ms(500);
    }
    wdt_clear();
    sensors_read_raw(&raw);
    accel_axes[1] = get_greatest_axis(&raw);
    compass_axes[1] = get_compass_axis(accel_axes[1]);
    wdt_clear();
    display_clear_screen();
    display_write_multiline(1, "Please place on\nedge with the\nlaser pointing\nleft", &small_font);
    for (i=0; i< 6; ++i) {
        wdt_clear();
        delay_ms(500);
    }
    wdt_clear();
    sensors_read_raw(&raw);
    accel_axes[0] = get_greatest_axis(&raw);
    compass_axes[0] = get_compass_axis(accel_axes[0]);
    display_clear_screen();
    wdt_clear();
    snprintf(text,18, "%d %d %d", accel_axes[0], accel_axes[1], accel_axes[2]);
    display_write_text(1,0,text, &small_font, false);
    wdt_clear();
    delay_ms(500);
    wdt_clear();
    snprintf(text,18, "%d %d %d", compass_axes[0], compass_axes[1], compass_axes[2]);
    display_write_text(4,0,text, &small_font, false);
    wdt_clear();
    
}

void apply_matrix_to_readings(vectorr src[], vectorr dest[], int reading_count, matrixx matrix) {
    int i;
    for (i=0; i< reading_count; i++) {
        apply_matrix(src[i], matrix, dest[i]);
        wdt_clear();
    }
}

void get_data_stats(vectorr readings[], int axes[2], int reading_count, accum offset[2], accum max_data[2], accum min_data[2]) {
    int i, k;
    for (k=0; k<2; k++) {
        min_data[k] = 0;
        max_data[k] = 0;
    }
    for (i=0; i<reading_count; i++) {
        for (k=0; k<2; k++) {
            min_data[k] = MIN(min_data[k],readings[i][axes[k]]);
            max_data[k] = MAX(max_data[k],readings[i][axes[k]]);
        }
        wdt_clear();
    }
    offset[0] =(min_data[0]+max_data[0])/2.0k;
    offset[1] =(min_data[1]+max_data[1])/2.0k;
}


void show_data(vectorr readings[], int axes[2], int reading_count) {
    accum offset[2], max_data[2], min_data[2];
    accum scale;
    int i, x, y;
    char text[20];
    display_clear_screen();
    get_data_stats(readings, axes, reading_count, offset, max_data, min_data);
    scale = MIN(32.0k / (max_data[0]-offset[0]),
                32.0k / (max_data[1]-offset[1]));
    for (i=0; i<reading_count; i++) {
        x = 64+(int)((readings[i][axes[0]]-offset[0])*scale);
        y = 32+(int)((readings[i][axes[1]]-offset[1])*scale);
        display_setbuffer_xy(x,y);
        wdt_clear();
    }
    x = 64+(int)((0-offset[0])*scale);
    y = 32+(int)((0-offset[1])*scale);
    display_draw_line(x-3,y,x+3,y);
    display_draw_line(x,y-3,x,y+3);
    display_show_buffer();
}

void show_modified_readings(vectorr readings[], int axes[2], int reading_count, matrixx matrix) {
    vectorr modified_readings[400];
    apply_matrix_to_readings(readings, modified_readings, reading_count, matrix);
    show_data(modified_readings,axes, reading_count);
    wdt_clear();
    delay_ms(800);
    wdt_clear();
}

void quick_cal(int32_t a) {
    struct COOKED_SENSORS sensors;
    int i, j, k;
    int readings_count;
    double x, y;
    accum min_separation = 30000k;
    accum gyro_offset = 0k;
    accum gyro = 0;
    accum last_gyro =0;
    vectorr readings[400];
    vectorr modified_readings[400];
    accum max_data[2] = {0k,0k};
    accum min_data[2] = {0k,0k};
    accum offset[2];
    accum scale;
    matrixx mag_matrix;
    char text[20];
    struct EIGEN eigen;
/* Brief summary of plan:
 * First place the device flat on the ground and leave alone
 * This allows us to calibrate zero-offsets for gyros*/
    memcpy(mag_matrix, identity, sizeof(matrixx));
    display_write_multiline(0, "Place device on a\nlevel surface\nand leave alone", &small_font);
    for (i=0; i<1; i++) {
        delay_ms(500);
        wdt_clear();
    }
    for (i=0; i<10; i++) {
        sensors_read_uncalibrated(&sensors);
        gyro_offset += sensors.gyro[2];
        wdt_clear();
    }
    gyro_offset /= 10.0k;
    display_write_multiline(0, "Rotate clockwise\n360' while\nleaving display\nfacing up", &small_font);
/* Now rotate around z-axis
 * do first magnetic calibration */
/* read in 400 or so readings while rotating */
    i = 0;
    do {
        do {
            sensors_read_uncalibrated(&sensors);
            gyro += ((sensors.gyro[2]-gyro_offset)*210)/1000k;
            delay_ms(20);
            wdt_clear();
        } while (abs(gyro)<i);
        readings[i][0] = sensors.mag[0];
        readings[i][1] = sensors.mag[1];
        readings[i][2] = sensors.mag[2];
        ++i;
        if (i==2) display_clear_screen();
        x = (int)(cos(-gyro*M_PI/180k)*30.0)+64;
        y = (int)(sin(-gyro*M_PI/180k)*30.0)+32;
        display_setbuffer_xy(x,y);
        display_show_buffer();
        wdt_clear();
    } while (abs(gyro)<400);
    readings_count = i;
    for (j = i/2; j< i; ++j) {
        if (min_separation>distance2(readings[0], readings[j])) {
            min_separation = distance2(readings[0], readings[j]);
            readings_count = j;
                    
        }   
    }
    show_modified_readings(readings,(int[2]){0,1}, readings_count, mag_matrix);
    get_data_stats(readings,(int[2]){0,1},readings_count, offset, min_data, max_data);
    apply_offset(-offset[0], -offset[1], 0, mag_matrix);
    show_modified_readings(readings,(int[2]){0,1}, readings_count, mag_matrix);
    apply_matrix_to_readings(readings, modified_readings, readings_count, mag_matrix);
    pca(modified_readings,(int[2]){0,1}, readings_count, &eigen);
    apply_2d_rotation((int[2]){0,1}, eigen.vector, mag_matrix);

    show_modified_readings(readings,(int[2]){0,1}, readings_count, mag_matrix);

    apply_scale(1,(accum)eigen.scalar, mag_matrix);
    
    show_modified_readings(readings,(int[2]){0,1}, readings_count, mag_matrix);

    eigen.vector[1] *= -1;
    apply_2d_rotation((int[2]){0,1}, eigen.vector, mag_matrix);

    show_modified_readings(readings,(int[2]){0,1}, readings_count, mag_matrix);
    memcpy(config.calib.mag, mag_matrix, sizeof(matrixx));
    wdt_clear();
    config_save();
    wdt_clear();
/* Now rotate around y-axis, keeping laser fixed on one point
 * this allows us to first calibrate magnetics and then calculate
 * direction of laser beam.
 */
 /* nor rotate around z-axis - allows calibration of y axis accelerometer */
}
void laser_cal() {
//	int count,x,y;
//	struct RAW_SENSORS raw;
//	short int mag_buffers[400][3];
//	//first get 400 data points with display offx
//	display_on(false);
//	laser_on(false);
//	__delay_ms(1000);
//	count=0;
//	for (count=0; count<100; ++count) {
//		sensors_read_raw(&raw,false);
//		memcpy(mag_buffers[count],raw.mag,6);
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	display_on(false);
//	laser_on(true);
//	__delay_ms(1000);
//	count=0;
//	for (count=100; count<200; ++count) {
//		sensors_read_raw(&raw,false);
//		memcpy(mag_buffers[count],raw.mag,6);
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	display_on(true);
//	laser_on(false);
//	__delay_ms(1000);
//	count=0;
//	for (count=200; count<300; ++count) {
//		sensors_read_raw(&raw,false);
//		memcpy(mag_buffers[count],raw.mag,6);
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	display_on(true);
//	laser_on(true);
//	__delay_ms(1000);
//	count=0;
//	for (count=300; count<400; ++count) {
//		sensors_read_raw(&raw,false);
//		memcpy(mag_buffers[count],raw.mag,6);
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	write_eeprom(0x200,mag_buffers,2400);
//
}


void full_cal() {
//	struct RAW_SENSORS sensors;
//	struct COOKED_SENSORS cooked;
//	float degrees=0.0;
//	float deg_cal=0.0;
//	float deg_limit_p = 1.0;
//	float deg_limit_m = -1.0;
//	float nearest = 100000.0;
//	float distance;
//	
//	double mag_buffers[400][3];
//	int count, len;
//	int complete = 0;
//	//wait 2 seconds
//	__delay_ms(2000);
//	//beep
//	//average gyro readings over 1s
//	for (count=0;count<100;count++) {
//		sensors_read_cooked(&cooked,false);
//		deg_cal += cooked.gyro[1];
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	deg_cal /= 100.0;
//	beep_on(4000);
//	__delay_ms(200);
//	beep_off();
//	//record 400 readings 
//	count=0;
//	while (count < 400) {
//		sensors_read_cooked(&cooked,false);
//		degrees += (cooked.gyro[1]-deg_cal)/100.0;
//		if ((degrees>deg_limit_p)||(degrees<deg_limit_m)) {
//			deg_limit_p += 1.0;
//			deg_limit_m -= 1.0;
//			memcpy(mag_buffers[count],sensors.mag,6);
//			count +=1;
//		}
//		while (!PORT_SENSOR_INT) __delay_ms(1);
//	}
//	// find closest point
//	for (count=320;count<400;count++) {
//		distance = (mag_buffers[count][0]-mag_buffers[0][0])*(mag_buffers[count][0]-mag_buffers[0][0]) +
//				   (mag_buffers[count][1]-mag_buffers[0][1])*(mag_buffers[count][1]-mag_buffers[0][1]) +
//				   (mag_buffers[count][2]-mag_buffers[0][2])*(mag_buffers[count][2]-mag_buffers[0][2]);
//		if (distance < nearest) {
//			len = count;
//			nearest = distance;
//		}
//	}
//	// do pca...
//	//beep again
//	beep_on(4000);
//	__delay_ms(200);
//	beep_off();
//	//copy data to EEPROM
//	write_eeprom(0x200,mag_buffers,2160);
//	__delay_ms(100);
//	beep_on(4000);
//	__delay_ms(200);
//	beep_off();
}
