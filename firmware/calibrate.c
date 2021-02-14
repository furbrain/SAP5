#include "config.h"
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>
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
#include "input.h"
#include "menu.h"
#include "ui.h"
#include "exception.h"
#include "mcc_generated_files/mcc.h"

GSL_MATRIX_DECLARE(mag_readings, CALIBRATION_SAMPLES + MAG_EXTRA_SAMPLES, 3);
GSL_MATRIX_DECLARE(grav_readings, CALIBRATION_SAMPLES, 3);
GSL_VECTOR_DECLARE(reading_weights, CALIBRATION_SAMPLES + MAG_EXTRA_SAMPLES);
double mag_cal_store[0x680/sizeof(double)] PLACE_DATA_AT(APP_CALIBRATION_LOCATION) = {0};
double grav_cal_store[0x180/sizeof(double)] PLACE_DATA_AT(APP_CALIBRATION_LOCATION+0x680) = {0};

DECLARE_MENU(calibration_menu, {
    /* calibrate menu */
    {"Sensors", Action, {calibrate_sensors}, 0},
    {"Laser", Action, {calibrate_laser}, 0},
    {"Axes", Action, {calibrate_axes}, 0},
    {"Back", Back, {NULL}, 0},
});


static
int get_greatest_accel_axis(struct RAW_SENSORS *raw) {
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

static
int get_greatest_mag_axis(struct RAW_SENSORS *a, struct RAW_SENSORS *b) {
    int max_abs = 0;
    int val;
    int axis = 0;
    int i;
    for (i=0; i<3; i++) {
        val = a->mag[i] - b->mag[i];
        if (abs(val)> max_abs) {
            if (val > 0) {
                axis = i;
            } else {
                axis = i+3;
            }
            max_abs = abs(val);
        }
    }   
    return axis;
}

static
void get_accel_axis(int i, const char *instruction) {
    struct RAW_SENSORS raw;
    display_write_multiline(0, instruction, true);
    delay_ms_safe(3000);
    if (get_clicks()!=NONE) {
        THROW_WITH_REASON("Axis calibration aborted", ERROR_PROCEDURE_ABORTED);
    }
    sensors_read_raw(&raw);
    config.axes.accel[i] = get_greatest_accel_axis(&raw);
}

static
void get_mag_axis(int i, const char *instructions[]) {
    struct RAW_SENSORS raw_a;
    struct RAW_SENSORS raw_b;
    display_write_multiline(0, instructions[0], true);
    delay_ms_safe(3000);
    if (get_clicks()!=NONE) {
        THROW_WITH_REASON("Axis calibration aborted", ERROR_PROCEDURE_ABORTED);
    }
    sensors_read_raw(&raw_a);
    display_write_multiline(0, instructions[1], true);
    delay_ms_safe(3000);
    if (get_clicks()!=NONE) {
        THROW_WITH_REASON("Axis calibration aborted", ERROR_PROCEDURE_ABORTED);
    }
    sensors_read_raw(&raw_b);
    config.axes.mag[i] = get_greatest_mag_axis(&raw_a, &raw_b);
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

void calibrate_axes(int32_t dummy) {
    int i;
    char text[18];
    const char *accel_instructions[] = {
        "Please place on\nedge with the\nlaser pointing\nleft",
        "Please point the\nlaser upwards",
        "Please place the\ndisplay flat on\na level surface"
    };
    const char *mag_instructions[] = {
        "Place flat with\nthe laser going\nleft and the top\nedge towards north",
        "Rotate 180'",
        "Please point the\nlaser north",
        "Rotate 180'",
        "Place on end\nwith the laser\ndown and display\n facing north",
        "Rotate 180'",
    };
    if (!ui_yes_no("Calib.\nAxes?")) return;
    for (i=2; i>=0; i--) {
        get_accel_axis(i, accel_instructions[i]);
    }
    for (i=0; i<3; i++) {
        get_mag_axis(i, &mag_instructions[i*2]);
    }
    if (!check_sane_axes()) {
        display_write_multiline(0, "Invalid axes\nfound.\nAborting", true);
        return;
    }
    display_clear(false);
    snprintf(text,18, "%d %d %d", config.axes.accel[0], config.axes.accel[1], config.axes.accel[2]);
    display_write_text(1,0,text, &small_font, false);
    snprintf(text,18, "%d %d %d", config.axes.mag[0], config.axes.mag[1], config.axes.mag[2]);
    display_write_text(4,0,text, &small_font, false);
    display_show_buffer();
    delay_ms_safe(6000);
    config_save();
    
}


double check_calibration(const gsl_matrix *data, calibration *cal) {
    GSL_VECTOR_DECLARE(result,3);
    unsigned int k;
    double magnitude;
    double error=0;
    for (k=0; k < data->size1; k++) {
        gsl_vector_const_view row = gsl_matrix_const_row(data, k);
        apply_calibration(&row.vector, cal, &result);
        magnitude = gsl_blas_dnrm2(&result);
        error += fabs(magnitude-1.0);
    }
    return (error/data->size1)*100;
}

double check_accuracy(const gsl_matrix *mag, const calibration *mag_cal,
                      const gsl_matrix *grav, const calibration *grav_cal) {
    GSL_VECTOR_DECLARE(mag_row, 3);
    GSL_VECTOR_DECLARE(grav_row, 3);
    GSL_VECTOR_DECLARE(deviation, 3);
    GSL_MATRIX_DECLARE(orientation, 8, 3);
    int i, j;
    int rotation_count = mag->size1 / 8;
    double result = 0;
    /* create matrix with vectors of orientation*/
    for (j=0; j< rotation_count; j++) {
        for (i=0; i<8; i++) {
            gsl_vector_const_view mag_temp = gsl_matrix_const_row(mag, i+j*8);
            gsl_vector_const_view grav_temp = gsl_matrix_const_row(grav, i+j*8);
            apply_calibration(&mag_temp.vector, mag_cal, &mag_row);
            apply_calibration(&grav_temp.vector, grav_cal, &grav_row);
            gsl_vector_view orient_row = gsl_matrix_row(&orientation, i);
            maths_get_orientation_as_vector(&mag_row,
                                            &grav_row,
                                            &orient_row.vector);
        }
        /* calculate absolute deviation for each axis*/
        for (i=0; i<3; i++) {
            gsl_vector_view column = gsl_matrix_column(&orientation, i);
            gsl_vector_set(&deviation, i, gsl_stats_absdev(column.vector.data, 
                                                        column.vector.stride,
                                                        column.vector.size));
        }

        result += gsl_blas_dnrm2(&deviation) * 180.0 / M_PI;
    }
    result /= rotation_count;
    return result;
}

void collect_mag_data(gsl_matrix *mag_data, gsl_vector *weights, double gyro_zero[3], int axis) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag_sensors = gsl_vector_view_array(sensors.mag,3);
    double angle = 0;
    double angle_wanted = MAG_DEGS_PER_SAMPLE;
    unsigned int elapsed,old;
    double time;
    int count = 0;
    laser_on();
    display_off();
    _CP0_SET_PERFCNT0_CONTROL(0);
    _CP0_SET_PERFCNT0_COUNT(0);
    old = 0.0;
    while (fabs(angle) < 360.0) {
        delay_ms_safe(10);
        sensors_read_uncalibrated(&sensors, 1);
        elapsed = _CP0_GET_PERFCNT0_COUNT();
        time = (elapsed-old)*1.0/_XTAL_FREQ;
        old=elapsed;
        angle += (sensors.gyro[axis] - gyro_zero[axis]) * time;
        if (fabs(angle) > angle_wanted) {
            angle_wanted += MAG_DEGS_PER_SAMPLE;
            gsl_matrix_set_row(mag_data, count, &mag_sensors.vector);
            gsl_vector_set(weights, count, 1);
            count++;
        }
    }
    beep_happy();
    laser_off();
    display_on();
}

void collect_data(gsl_matrix *mag_data, gsl_matrix *grav_data, gsl_vector *weights) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag_sensors = gsl_vector_view_array(sensors.mag,3);
    gsl_vector_view grav_sensors = gsl_vector_view_array(sensors.accel,3);
    unsigned int i;
    display_off();
    laser_on();
    for (i=0; i< mag_data->size1; i++) {
        //delay to let user move to position
        if (get_clicks()!=NONE) {
            THROW_WITH_REASON("Calibration aborted", ERROR_PROCEDURE_ABORTED);
        }
        if (i==0) {
            delay_ms_safe(2000);
        } else {
            delay_ms_safe(4000);
        }
        if (get_clicks()!=NONE) {
            THROW_WITH_REASON("Calibration aborted", ERROR_PROCEDURE_ABORTED);
        }
        //read in samples
        sensors_read_uncalibrated(&sensors, SAMPLES_PER_READING);
        gsl_matrix_set_row(mag_data, i, &mag_sensors.vector);
        gsl_matrix_set_row(grav_data, i, &grav_sensors.vector);
        gsl_vector_set(weights, i, SAMPLES_PER_READING);
        //beep to let user know to move to next position.
        laser_off();
        delay_ms_safe(300);
        beep_beep();
        laser_on();
        timeout_reset();
    }
    beep_happy();
    laser_off();
    display_on();
    display_clear(true);
}

void get_calibration_data(gsl_matrix *mag, gsl_matrix *grav) {
    int count;
    int row;
    struct COOKED_SENSORS sensors;
    gsl_matrix_view mag_view;
    gsl_matrix_view grav_view;
    gsl_vector_view weight_view;
    const char *mag_rotate_instructions[] = {
        "Press the button\nthen rotate\nend over end\nuntil the beep",
        "Press the button\nthen rotate\nside over side\nuntil the beep",
        "Press the button\nthen rotate\nahorizontally\nuntil the beep",
    };
    const char *stepped_rotate_instructions[] = {
        "Place device on\ninclined surface\nand press button",
        "After each beep\nrotate by ~90'",
        "Place device flat\nand press button",
        "After each beep\nrotate end over\nend by ~90'"
    };
    
    /* get continuous readings in 3 axes*/
    display_write_multiline(0, "Place device on\n"
                               "a flat surface\n"
                               "and press button", true);
    get_single_click_or_throw("Calibration aborted", ERROR_PROCEDURE_ABORTED);
    delay_ms_safe(100);
    sensors_read_uncalibrated(&sensors, 50);
    for (count = 0; count< 3; count++) {
        display_write_multiline(0, mag_rotate_instructions[count], true);
        get_single_click_or_throw("Calibration aborted", ERROR_PROCEDURE_ABORTED);
        row = CALIBRATION_SAMPLES + count * MAG_SAMPLES_PER_AXIS;
        mag_view = gsl_matrix_get_rows(mag, row, MAG_SAMPLES_PER_AXIS);
        weight_view = gsl_vector_subvector(&reading_weights, row, MAG_SAMPLES_PER_AXIS);
        collect_mag_data(&mag_view.matrix, &weight_view.vector, sensors.gyro, count);
    }
    
    /* get static readings around x and z axes*/
    for(count=0; count < 2; count++) {
        display_write_multiline(0, stepped_rotate_instructions[count*2], true);
        get_single_click_or_throw("Calibration aborted", ERROR_PROCEDURE_ABORTED);
        display_write_multiline(0, stepped_rotate_instructions[count*2+1], true);
        row = count * CAL_AXIS_COUNT;
        mag_view = gsl_matrix_get_rows(mag, row, CAL_AXIS_COUNT);
        grav_view = gsl_matrix_get_rows(grav, row, CAL_AXIS_COUNT);
        weight_view = gsl_vector_subvector(&reading_weights, row, CAL_AXIS_COUNT);
        collect_data(&mag_view.matrix, &grav_view.matrix, &weight_view.vector);
    }
    
    /*get static readings around y-axis */
    for (count=0; count<2; count++) {
        laser_on();
        display_write_multiline(0, "Point laser at\n"
                                   "fixed target\n"
                                   "and press button", true);
        get_single_click_or_throw("Calibration aborted", ERROR_PROCEDURE_ABORTED);
        display_write_multiline(0, "After each beep\n"
                                   "rotate by ~45'\n"
                                   "leaving laser\n"
                                   "on target", true);
        delay_ms_safe(3000);
        row  = CAL_AXIS_COUNT*2 + CAL_TARGET_COUNT * count;
        mag_view = gsl_matrix_get_rows(mag, row, CAL_TARGET_COUNT);
        grav_view = gsl_matrix_get_rows(grav, row, CAL_TARGET_COUNT);
        weight_view = gsl_vector_subvector(&reading_weights, row, CAL_TARGET_COUNT);
        collect_data(&mag_view.matrix, &grav_view.matrix, &weight_view.vector);
    }
}

void calibrate_sensors(int32_t dummy) {
    char text[30];
    CALIBRATION_DECLARE(grav_cal);
    CALIBRATION_DECLARE(mag_cal);
    gsl_matrix_const_view mag_spins = gsl_matrix_const_submatrix(&mag_readings, 
            CAL_AXIS_COUNT*2, 0, 
            CAL_TARGET_COUNT*2, 3);
    gsl_matrix_const_view grav_spins = gsl_matrix_const_submatrix(&grav_readings, 
            CAL_AXIS_COUNT*2, 0, 
            CAL_TARGET_COUNT*2, 3);
    double grav_error, mag_error, accuracy;
    /* get data */
    if (!ui_yes_no("Calib.\nSensors?")) return;
    get_calibration_data(&mag_readings, &grav_readings);
    memory_erase_page(mag_cal_store);
    memory_write_data(mag_cal_store, mag_readings_data, sizeof(mag_readings_data));
    memory_write_data(grav_cal_store, 
                      grav_readings_data, sizeof(grav_readings_data));
    display_write_multiline(0, "Processing", true);    
    //do calibration    
    fit_ellipsoid(&mag_readings, &reading_weights, &mag_cal);
    fit_ellipsoid(&grav_readings, &reading_weights,  &grav_cal);
    align_all_sensors(&mag_spins.matrix, &mag_cal, &grav_spins.matrix, &grav_cal);
    get_shear(&mag_spins.matrix, &mag_cal, &grav_spins.matrix, &grav_cal);
    
    
    // show mag error
    display_clear(true);
    mag_error = check_calibration(&mag_readings, &mag_cal);
    sprintf(text, "Mag Err:  %.2f%%", mag_error);
    display_write_multiline(2, text, false);
    display_show_buffer();
    
    //show grav error
    grav_error = check_calibration(&grav_readings, &grav_cal);
    sprintf(text, "Grav Err: %.2f%%", grav_error);
    display_write_multiline(4, text, false);
    display_show_buffer();
    
    //show accuracy
    accuracy = check_accuracy(&mag_spins.matrix, &mag_cal,
                              &grav_spins.matrix, &grav_cal);
    sprintf(text, "Accuracy: %.2f`", accuracy);
    display_write_multiline(6, text, false);
    display_show_buffer();
    delay_ms_safe(4000);
    //save calibration data
    
    //check satisfactory calibration
    if (isnan(grav_error) || isnan(mag_error) || isnan(accuracy)) {
        display_write_multiline(2, "Calibration failed\nNot Saved", true);
    } else if ((grav_error > 5.0) || (mag_error > 5.0) || (accuracy > 1.2)) {
        display_write_multiline(2, "Poor calibration\nNot saved", true);
    } else {
        if (ui_yes_no("Cal. Good\nSave?")) {
            calibration conf_grav = calibration_from_doubles(config.calib.accel);
            calibration conf_mag = calibration_from_doubles(config.calib.mag);
            calibration_memcpy(&conf_grav, &grav_cal);
            calibration_memcpy(&conf_mag, &mag_cal);
            wdt_clear();
            config_save();
        }
    }
}

void calibrate_laser(int32_t dummy) {
    GSL_VECTOR_DECLARE(samples, 10);
    size_t i;
    double distance, error, offset;
    char text[80];
    display_write_multiline(0,"Place a target 1m\n"
                              "from the rearmost\n"
                              "point of the\n"
                              "device", true);
    if (!ui_yes_no("Calib.\nLaser?")) return;
    laser_on();
    delay_ms_safe(4000);
    for (i=0; i<samples.size; ++i) {
        laser_on();
        delay_ms_safe(100);
        if (get_clicks()!=NONE) {
            THROW_WITH_REASON("Axis calibration aborted", ERROR_PROCEDURE_ABORTED);
        }
        distance = laser_read_raw(LASER_MEDIUM, 4000);
        gsl_vector_set(&samples, i, distance);
        beep_beep();
    }
    laser_off();
    distance = gsl_stats_mean(samples.data, samples.stride, samples.size);
    error = gsl_stats_sd(samples.data, samples.stride, samples.size);
    offset = 1.000 - distance;
    config.calib.laser_offset = offset;
    sprintf(text, "Offset: %.3f\n"
                  "Error: %.3f\n"
                  "Config saved", offset, error);
    display_clear(false);
    display_write_multiline(0, text, false);
    display_show_buffer();
    delay_ms_safe(2000);
    config_save();
    display_write_multiline(6, "Saved", true);
    display_show_buffer();
    delay_ms_safe(1000);
}

