#include "config.h"
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_sort_vector.h>
#include "display.h"
#include "font.h"
#include "sensors.h"
#include "maths.h"
#include "leg.h"
#include "calibrate.h"
#include "utils.h"
#include "laser.h"
#include "gsl_static.h"
#include "beep.h"

GSL_MATRIX_DECLARE(mag_readings, CALIBRATION_SAMPLES, 3);
GSL_MATRIX_DECLARE(grav_readings, CALIBRATION_SAMPLES, 3);
GSL_MATRIX_DECLARE(temp_mag_readings, CALIBRATION_SUB_SAMPLES, 3);
GSL_MATRIX_DECLARE(temp_grav_readings, CALIBRATION_SUB_SAMPLES, 3);

static
int get_greatest_axis(struct RAW_SENSORS *raw) {
    int max_abs = 0;
    int axis = 0;
    int i;
    for (i=0; i<3; i++) {
        if (abs(raw->accel[i])> max_abs) {
            if (raw->accel[i] < 0) {
                axis = i;
            } else {
                axis = i+3;
            }
            max_abs = abs(raw->accel[i]);
        }
    }   
    return axis;
}

const int compass_axis_map[] = {4,3,2,1,0,5};

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
        display_clear_screen(true);
        display_write_multiline(0, instructions[i], true);
        delay_ms_safe(3000);
        set_axis(i);
    }
    if (!check_sane_axes()) {
        display_clear_screen(true);
        display_write_multiline(0, "Invalid axes\nfound.\nAborting", true);
        return;
    }
    snprintf(text,18, "%d %d %d", config.axes.accel[0], config.axes.accel[1], config.axes.accel[2]);
    display_write_text(1,0,text, &small_font, false, true);
    snprintf(text,18, "%d %d %d", config.axes.mag[0], config.axes.mag[1], config.axes.mag[2]);
    display_write_text(4,0,text, &small_font, false, true);
    delay_ms_safe(6000);
    config_save();
    
}


double check_calibration(const gsl_matrix *data, int len, matrixx calibration) {
    vectorr vector, v_result;
    int k;
    double magnitude;
    double max_error=0;
    for (k=0; k<len; k++) {
        vector[0] = gsl_matrix_get(data,k,0);
        vector[1] = gsl_matrix_get(data,k,1);
        vector[2] = gsl_matrix_get(data,k,2);
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

int collect_data_around_axis(gsl_matrix *mag_data, gsl_matrix *grav_data, int offset) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag_sensors = gsl_vector_view_array(sensors.mag,3);
    gsl_vector_view grav_sensors = gsl_vector_view_array(sensors.accel,3);
    gsl_vector_view samples;
    gsl_vector_view median;
    int i, j;
    for (i=0; i< 8; i++) {
        //delay to let user move to position
        delay_ms_safe(5000);
        //read in samples
        for (j=0; j<CALIBRATION_SUB_SAMPLES; j++) {
            sensors_read_uncalibrated(&sensors);
            gsl_matrix_set_row(&temp_mag_readings, j, &mag_sensors.vector);
            gsl_matrix_set_row(&temp_grav_readings, j, &grav_sensors.vector);
        }
        //sort our samples
        for (j=0; j<3; j++){
            samples = gsl_matrix_column(&temp_mag_readings,j);
            gsl_sort_vector(&samples.vector);
            samples = gsl_matrix_column(&temp_grav_readings,j);
            gsl_sort_vector(&samples.vector);
        }
        //append median of all sample data to the set of readings
        median = gsl_matrix_row(&temp_mag_readings,CALIBRATION_SUB_SAMPLES/2);
        gsl_matrix_set_row(mag_data, i+offset, &median.vector);
        median = gsl_matrix_row(&temp_grav_readings,CALIBRATION_SUB_SAMPLES/2);
        gsl_matrix_set_row(grav_data, i+offset, &median.vector);
        //beep to let user know to move to next position.
        laser_off();
        delay_ms(300);
        beep_beep();
        laser_on();
    }
    beep_happy();
    return 8;
}
//    do {
//        do {
//            sensors_read_uncalibrated(&sensors);
//            gyro += ((sensors.gyro[axis]-gyro_offset)*20)/1000;
//            delay_ms_safe(20);
//        } while (fabs(gyro)<(i*3));
//        for (j=0; j<3; j++) {
//           mag_data[i*3+j] = sensors.mag[j];
//           grav_data[i*3+j] = sensors.accel[j];
//        }
//        i++;
//    } while ((fabs(gyro)<400) && i < (CALIBRATION_SAMPLES/2));

void calibrate_sensors(int32_t a) {
    char text[30];
    matrixx accel_mat, mag_mat;
    int z_axis_count, y_axis_count, data_length;
    double grav_error, mag_error;
/* Brief summary of plan:
 * First place the device flat on the ground and leave alone
 * This allows us to calibrate zero-offsets for gyros*/
    display_clear_screen(true);
    display_write_multiline(0, "Place device on a\nlevel surface\nand leave alone", true);
    delay_ms_safe(2000);
    //gyro_offset = get_gyro_offset(2);
    display_clear_screen(true);
    display_write_multiline(0, "After each beep\nrotate by ~45'\nleaving display\nfacing up", true);
    delay_ms_safe(1500);
    beep_beep();
    /* Now rotate around z-axis and read in ~CALIBRATION_SAMPLES/2 readings */
    laser_on();
    display_off();
    z_axis_count = collect_data_around_axis(&mag_readings, &grav_readings, 0);
    wdt_clear();
    
    /* now read data on y-axis */
    display_on();
    display_clear_screen(true);
    display_write_multiline(0, "Point laser at\nfixed target", true);
    delay_ms_safe(2000);
    //gyro_offset = get_gyro_offset(1);
    display_clear_screen(true);
    display_write_multiline(0, "After each beep\nrotate by ~45'\nleaving laser\non target", true);
    delay_ms_safe(1500);
    display_off();
    laser_on();
    y_axis_count = collect_data_around_axis(&mag_readings, &grav_readings, z_axis_count);
    data_length = z_axis_count + y_axis_count;
    wdt_clear();
    laser_off();
    display_on();
    display_clear_screen(true);
    display_write_multiline(0, "Processing", true);
    delay_ms_safe(2000);
    // calibrate magnetometer
    write_data(leg_store.raw, &mag_readings_data, CALIBRATION_SAMPLES*3*4);
    write_data(&leg_store.raw[0x800], &grav_readings_data, CALIBRATION_SAMPLES*3*4);
    calibrate(&mag_readings, data_length, mag_mat);
    mag_error = check_calibration(&mag_readings, data_length, mag_mat);
    sprintf(text, "Mag Err:  %.2f%%", mag_error);
    display_write_multiline(2,text, true);
    wdt_clear();
    // calibrate accelerometer
    calibrate(&grav_readings, data_length, accel_mat);
    grav_error = check_calibration(&grav_readings, data_length, accel_mat);
    sprintf(text, "Grav Err: %.2f%%", grav_error);
    display_write_multiline(4,text, true);
    delay_ms_safe(4000);
    if (isnan(grav_error) || isnan(mag_error)) {
        display_write_multiline(2, "Calibration failed\nNot Saved", true);
    } else if ((grav_error > 5.0) || (mag_error > 5.0)) {
        display_write_multiline(2, "Poor calibration\nNot saved", true);
    } else {
        display_write_multiline(2, "Calibration Good\nSaved.", true);
        memcpy(config.calib.accel, accel_mat, sizeof(matrixx));
        memcpy(config.calib.mag, mag_mat, sizeof(matrixx));
        wdt_clear();
        config_save();
    }
    delay_ms_safe(4000);    
}

void laser_cal() {
}

