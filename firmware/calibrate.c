#include "config.h"
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "maths.h"
#include "storage.h"

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

static
int get_compass_axis(int axis) {
    return compass_axis_map[axis];
}

static
void set_axis(int i) {
    struct RAW_SENSORS raw;
    sensors_read_raw(&raw);
    config.axes.accel[i] = get_greatest_axis(&raw);
    config.axes.mag[i] = get_compass_axis(config.axes.accel[i]);
}

static
bool check_sane_axes(void) {
    int i,j;
    bool accel_error, mag_error;
    for (i=0; i<3; i++) {
        accel_error = mag_error = true;
        for (j=0; j<3; j++) {
            if ((config.axes.accel[j] %3) == i) accel_error = false;
            if ((config.axes.mag[j] %3) == i) mag_error = false;
        }
        if (accel_error || mag_error) return false;
    }
    return true;
}

void calibrate_axes(int dummy) {
    int i;
    char text[18];
    const char *instructions[] = {
        "Please place on\nedge with the\nlaser pointing\nleft",
        "Please point the\nlaser upwards",
        "Please place the\ndisplay flat on\na level surface"
    };
    for (i=2; i>=0; i--) {
        display_clear_screen();
        display_write_multiline(0, instructions[i], &small_font);
        delay_ms_safe(3000);
        set_axis(i);
    }
    if (!check_sane_axes()) {
        display_clear_screen();
        display_write_multiline(0, "Invalid axes\nfound.\nAborting", &small_font);
        return;
    }
    snprintf(text,18, "%d %d %d", config.axes.accel[0], config.axes.accel[1], config.axes.accel[2]);
    display_write_text(1,0,text, &small_font, false);
    snprintf(text,18, "%d %d %d", config.axes.mag[0], config.axes.mag[1], config.axes.mag[2]);
    display_write_text(4,0,text, &small_font, false);
    delay_ms_safe(500);
    config_save();
    
}

static
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

accum get_gyro_offset(int axis) {
    int i;
    accum gyro_offset = 0.0k;
    struct COOKED_SENSORS sensors;
    for (i=0; i<10; i++) {
        sensors_read_uncalibrated(&sensors);
        gyro_offset += sensors.gyro[2];
        wdt_clear();
    }
    gyro_offset /= 10.0k;
    return gyro_offset;
}

int collect_data_around_axis(int axis, accum gyro_offset, double *mag_data, double *grav_data) {
    struct COOKED_SENSORS sensors;
    accum gyro=0;
    int i, j, readings_count;
    double min_separation = 10000000.0;
    i = 0;
    do {
        do {
            sensors_read_uncalibrated(&sensors);
            gyro += ((sensors.gyro[axis]-gyro_offset)*20k)/1000k;
            delay_ms_safe(20);
        } while (aabs(gyro)<(i*3));
        for (j=0; j<3; j++) {
           mag_data[i*3+j] = sensors.mag[j];
           grav_data[i*3+j] = sensors.accel[j];
        }
        i++;
    } while ((aabs(gyro)<400) && i < 120);
    return i;
}

void calibrate_sensors(int32_t a) {
    double mag_readings[720]; //3 x240 sets of readings...
    double grav_readings[720];
    matrixx accel_mat, mag_mat;
    accum gyro_offset = 0k;
    int z_axis_count, y_axis_count, offset, data_length;
/* Brief summary of plan:
 * First place the device flat on the ground and leave alone
 * This allows us to calibrate zero-offsets for gyros*/
    display_clear_screen();
    display_write_multiline(0, "Place device on a\nlevel surface\nand leave alone", &small_font);
    delay_ms_safe(2000);
    gyro_offset = get_gyro_offset(2);
    display_clear_screen();
    display_write_multiline(0, "Rotate clockwise\n360' while\nleaving display\nfacing up", &small_font);
    delay_ms_safe(1500);
    /* Now rotate around z-axis and read in ~120 readings */
    laser_on(true);
    display_on(false);
    z_axis_count = collect_data_around_axis(2, gyro_offset, mag_readings, grav_readings);
    wdt_clear();
    
    /* now read data on y-axis */
    display_on(true);
    display_clear_screen();
    display_write_multiline(0, "Point laser at\nfixed target", &small_font);
    delay_ms_safe(2000);
    gyro_offset = get_gyro_offset(1);
    display_clear_screen();
    display_write_multiline(0, "Rotate device\n360' while\nleaving laser\non target", &small_font);
    delay_ms_safe(1500);
    offset = z_axis_count*3;
    display_on(false);
    laser_on(true);
    y_axis_count = collect_data_around_axis(1, gyro_offset, mag_readings+offset, grav_readings+offset);
    data_length = z_axis_count + y_axis_count;
    wdt_clear();
    laser_on(false);
    display_on(true);
    display_clear_screen();
    display_write_multiline(0, "Storing", &small_font);
    delay_ms_safe(1000);
    write_data((uint8_t*)leg_space, mag_readings, sizeof(mag_readings));
    write_data((uint8_t*)(leg_space+sizeof(mag_readings)), grav_readings, sizeof(grav_readings));
    display_write_multiline(2, "Processing", &small_font);
    delay_ms_safe(1000);
    // calibrate magnetometer
    calibrate(mag_readings, data_length, mag_mat);
    wdt_clear();
    // calibrate accelerometer
    calibrate(grav_readings, data_length, accel_mat);
    display_write_multiline(4, "Done", &small_font);
    delay_ms_safe(1000);
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
