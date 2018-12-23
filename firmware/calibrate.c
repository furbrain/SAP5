#include "config.h"
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "maths.h"
#include "leg.h"
#include "calibrate.h"

#define MIN(a,b) ((a)<(b))?(a):(b)
#define MAX(a,b) ((a)>(b))?(a):(b)
static double mag_readings[CALIBRATION_SAMPLES*3]; //3 sets of readings...
static double grav_readings[CALIBRATION_SAMPLES*3];

static
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


double check_calibration(double *data, int len, matrixx calibration) {
    vectorr vector, v_result;
    int k;
    double magnitude;
    double max_error=0;
    for (k=0; k<len; k++) {
        vector[0] = data[k*3];
        vector[1] = data[k*3+1];
        vector[2] = data[k*3+2];
        apply_matrix(vector, calibration, v_result);
        magnitude = v_result[0]*v_result[0] + v_result[1]*v_result[1] + v_result[2]*v_result[2];
        max_error += fabs(magnitude-1.0);
    }
    return (max_error/len)*100;
}

double get_gyro_offset(int axis) {
    int i;
    double gyro_offset = 0.0;
    struct COOKED_SENSORS sensors;
    for (i=0; i<10; i++) {
        sensors_read_uncalibrated(&sensors);
        gyro_offset += sensors.gyro[2];
        wdt_clear();
    }
    gyro_offset /= 10.0;
    return gyro_offset;
}

int collect_data_around_axis(int axis, double gyro_offset, double *mag_data, double *grav_data) {
    struct COOKED_SENSORS sensors;
    double gyro=0;
    int i, j, readings_count;
    double min_separation = 10000000.0;
    i = 0;
    do {
        do {
            sensors_read_uncalibrated(&sensors);
            gyro += ((sensors.gyro[axis]-gyro_offset)*20)/1000;
            delay_ms_safe(20);
        } while (fabs(gyro)<(i*3));
        for (j=0; j<3; j++) {
           mag_data[i*3+j] = sensors.mag[j];
           grav_data[i*3+j] = sensors.accel[j];
        }
        i++;
    } while ((fabs(gyro)<400) && i < (CALIBRATION_SAMPLES/2));
    return i;
}

void calibrate_sensors(int32_t a) {
    char text[30];
    matrixx accel_mat, mag_mat;
    double gyro_offset = 0;
    int z_axis_count, y_axis_count, offset, data_length;
    double error;
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
    /* Now rotate around z-axis and read in ~CALIBRATION_SAMPLES/2 readings */
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
    display_write_multiline(0, "Processing", &small_font);
    // calibrate magnetometer
    calibrate(mag_readings, data_length, mag_mat);
    error = check_calibration(mag_readings, data_length, mag_mat);
    sprintf(text, "Mag Err:  %.2f%%", error);
    display_write_multiline(2,text, &small_font);
    wdt_clear();
    // calibrate accelerometer
    calibrate(grav_readings, data_length, accel_mat);
    error = check_calibration(grav_readings, data_length, accel_mat);
    sprintf(text, "Grav Err: %.2f%%", error);
    display_write_multiline(4,text, &small_font);
    memcpy(config.calib.accel, accel_mat, sizeof(matrixx));
    memcpy(config.calib.mag, mag_mat, sizeof(matrixx));
    wdt_clear();
    config_save();
    display_write_multiline(6, "Done", &small_font);
    delay_ms_safe(4000);
    
}

void laser_cal() {
}

