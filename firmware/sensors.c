#define USE_AND_OR
#include <string.h>
#include "config.h"
#include "sensors.h"
#include "display.h"
#include "i2c_util.h"
#include "maths.h"
#include "mcc_generated_files/rtcc.h"
#include "utils.h"
#include "exception.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_statistics.h>
#include "gsl_static.h"
#include "version.h"

GSL_MATRIX_DECLARE(temp_mag_readings, SAMPLES_PER_READING, 3);
GSL_MATRIX_DECLARE(temp_grav_readings, SAMPLES_PER_READING, 3);

#define length(a) sizeof(a)/sizeof(a[0])
#define send_multi(addr, commands) write_i2c_multi(addr, commands, length(commands))

#define MPU9250_ADDRESS 0x68
#define MPU9250_GYRO_FULL_SCALE 250
#define MPU9250_ACCEL_FULL_SCALE 2 
#define MPU9250_MAG_FULL_SCALE 4800
const uint8_t MPU9250_ACCEL_AXES[] = {4,0,5};
const uint8_t MPU9250_MAG_AXES[] = {0,4,5};
const uint8_t BM1422_MAG_AXES[] = {4,3,5};

#define BM1422_ADDRESS 0x0e
#define BM1422_MAG_FULL_SCALE 1376

float ACCEL_FULL_SCALE, MAG_FULL_SCALE, GYRO_FULL_SCALE;

const i2c_multi_commands MPU9250_init_commands[] = {
    {0x6A, 0},
    {0x6A,7},

    {0x19,9}, //gives sample rate of 100Hz
    {0x1A,0x42}, //FIFO Lockes, gyro 84Hz low-pass filter, no external sync
    {0x1B, 0}, //Gyro full scale 250 deg/s
    {0x1C, 0}, //Accel ful scale +/-2g
	{0x1D, 2}, //Accel 99Hz low-pass filter
    //set fifo enablement
	{0x37, 0x0}, //disable interrupt pin
	{0x38, 0}, //disable interrupt pin
    {0x23, 0x78}, //gyro+accel only
};

const i2c_multi_commands AK8963_commands[] = {

        //put i2c control stuff here
    {0x24, 0x40}, //I2C_MST_CTRL
    {0x25, 0x8C}, //SLV0_ADDR
    {0x26, 0x03}, //SLV0_REG
    {0x27, 0xD7}, //SLV0_CTRL
    {0x28, 0x0C}, //SLV1_ADDR
    {0x29, 0x0A}, //SLV1_REG
    {0x2A, 0x81}, //SLV1_CTRL
    {0x64, 0x11}, //SLV1_DO
    {0x34, 0x00}, //I2C_MST_DELAY
    {0x67, 0x83}, //I2C_MST_DELAY_CTRL  
    {0x23, 0x79}, //FIFO sensors - gyro, accel and i2c slave 0                    
};

const i2c_multi_commands MPU9250_reset_commands[] = {
    {0x6A, 0x67}, //USER_CTRL
    {0x6B, 0}, //PWR_MGMT_1    
};

const i2c_multi_commands BM1422_init_commands[] = {
    {0x1B, 0xC8}, //14bits, 100Hz read rate, continuous mode
    {0x5C, 0x00},
    {0x5D, 0x00}, //clear reset status
    {0x1C, 0x00}, //do not use Data ready pin
    {0x40, 0x10}, //average 16 readings together
    {0x1D, 0x40}, //set it going    
};


void sensors_init() {
    switch (version_hardware) {
        case VERSION_ALPHA:
            send_multi(MPU9250_ADDRESS, MPU9250_init_commands);
            send_multi(MPU9250_ADDRESS, AK8963_commands);
            send_multi(MPU9250_ADDRESS, MPU9250_reset_commands);
            ACCEL_FULL_SCALE = MPU9250_ACCEL_FULL_SCALE;
            GYRO_FULL_SCALE = MPU9250_GYRO_FULL_SCALE;
            MAG_FULL_SCALE = MPU9250_MAG_FULL_SCALE;
            if (config.axes.accel[0]>=5) {
                memcpy(config.axes.accel, MPU9250_ACCEL_AXES, sizeof(MPU9250_ACCEL_AXES));
                memcpy(config.axes.mag, MPU9250_MAG_AXES, sizeof(MPU9250_MAG_AXES));
            }
            break;
        case VERSION_V1_0:
            send_multi(MPU9250_ADDRESS, MPU9250_init_commands);
            send_multi(MPU9250_ADDRESS, MPU9250_reset_commands);
            send_multi(BM1422_ADDRESS, BM1422_init_commands);
            ACCEL_FULL_SCALE = MPU9250_ACCEL_FULL_SCALE;
            GYRO_FULL_SCALE = MPU9250_GYRO_FULL_SCALE;
            MAG_FULL_SCALE = BM1422_MAG_FULL_SCALE;
            if (config.axes.accel[0]>=5) {
                memcpy(config.axes.accel, MPU9250_ACCEL_AXES, sizeof(MPU9250_ACCEL_AXES));
                memcpy(config.axes.mag, BM1422_MAG_AXES, sizeof(BM1422_MAG_AXES));
            }
            break;
        case VERSION_V1_1:
            break;
        default:
            break;          
    }
}



void byte_swap(uint16_t *word){
    *word = (*word << 8 | *word >> 8);    
}

void sensors_read_raw(struct RAW_SENSORS *sensors){
    int i;
    switch (version_hardware) {
        case VERSION_ALPHA:
            if (read_i2c_data(MPU9250_ADDRESS, 0x3B, (uint8_t *)sensors, sizeof(*sensors))) {
                THROW_WITH_REASON("I2C communication failed", ERROR_MAGNETOMETER_FAILED);
            }
            for(i=0; i< 10; ++i) {
                byte_swap(&((uint16_t*)sensors)[i]);
            }
            break;
        case VERSION_V1_0:
            if (read_i2c_data(MPU9250_ADDRESS, 0x3B, (uint8_t *)sensors, 14)) {
                THROW_WITH_REASON("MPU9250 communication failed", ERROR_MAGNETOMETER_FAILED);
            }
            for(i=0; i< 7; ++i) {
                byte_swap(&((uint16_t*)sensors)[i]);
            }
            if (read_i2c_data(BM1422_ADDRESS, 0x10, (uint8_t *)sensors->mag, 6)) {
                THROW_WITH_REASON("BM1422 communication failed", ERROR_MAGNETOMETER_FAILED);
            }
            break;
        default:
            THROW_WITH_REASON("Bad Hardware", ERROR_HARDWARE_NOT_IDENTIFIED);
            break;
    }
}

void sensors_raw_adjust_axes(struct RAW_SENSORS *sensors){
    struct RAW_SENSORS temp_sensors;
    int i;
    int sign;
    int axis;
    memcpy(&temp_sensors, sensors, sizeof(temp_sensors));
    for (i=0;i<3; i++) {
        sign = config.axes.accel[i]>=3 ? -1 : 1;
        axis = config.axes.accel[i] % 3;
        sensors->accel[i] = temp_sensors.accel[axis] * sign;
        sensors->gyro[i] = temp_sensors.gyro[axis] * sign;
        sign = config.axes.mag[i]>=3 ? -1 : 1;
        axis = config.axes.mag[i] % 3;
        sensors->mag[i] = temp_sensors.mag[axis] * sign;
    }
}

void sensors_raw_to_uncalibrated(struct COOKED_SENSORS *cooked, struct RAW_SENSORS *raw){
    int i;
    //first correct axes and polarity
    sensors_raw_adjust_axes(raw);
    // first convert to doubles with sensible units
	// also account for vagaries of sensor alignment
    for (i=0; i<3; i++) {
        cooked->accel[i] = (raw->accel[i]/32768.0) * ACCEL_FULL_SCALE;
        cooked->gyro[i] = (raw->gyro[i]/32768.0) * GYRO_FULL_SCALE;
        cooked->mag[i] = (raw->mag[i]/32768.0) * MAG_FULL_SCALE;
    }
    cooked->temp = (raw->temp/333.87)+21.0;
}


void sensors_read_uncalibrated(struct COOKED_SENSORS *sensors, int count) {
    struct RAW_SENSORS raw_sensors;
    gsl_vector_view mag_sensors = gsl_vector_view_array(sensors->mag,3);
    gsl_vector_view grav_sensors = gsl_vector_view_array(sensors->accel,3);
    gsl_vector_view samples;
    GSL_MATRIX_RESIZE(temp_mag_readings, count , 3);
    GSL_MATRIX_RESIZE(temp_grav_readings, count , 3);
    int i;
    double average;
    for (i=0; i<count; ++i) {
        sensors_read_raw (&raw_sensors);
        sensors_raw_to_uncalibrated(sensors, &raw_sensors);
        gsl_matrix_set_row(&temp_mag_readings, i, &mag_sensors.vector);
        gsl_matrix_set_row(&temp_grav_readings, i, &grav_sensors.vector);
        delay_ms_safe(10);
    }
#if 0
    /* use median*/
    for (i=0; i<3; ++i){
        samples = gsl_matrix_column(&temp_mag_readings,i);
        gsl_sort_vector(&samples.vector);
        samples = gsl_matrix_column(&temp_grav_readings,i);
        gsl_sort_vector(&samples.vector);
    }
    gsl_matrix_get_row(&mag_sensors.vector, &temp_mag_readings, count/2);
    gsl_matrix_get_row(&grav_sensors.vector, &temp_grav_readings, count/2);
#else
    /* use mean */
    for (i=0; i<3; ++i) {
        samples = gsl_matrix_column(&temp_mag_readings,i);
        average = gsl_stats_mean(samples.vector.data, samples.vector.stride, samples.vector.size);
        sensors->mag[i] = average;
        samples = gsl_matrix_column(&temp_grav_readings,i);
        average = gsl_stats_mean(samples.vector.data, samples.vector.stride, samples.vector.size);
        sensors->accel[i] = average;
    }
#endif 
}

void sensors_uncalibrated_to_cooked(struct COOKED_SENSORS *cooked){
    GSL_VECTOR_DECLARE(temp_accel,3);
    GSL_VECTOR_DECLARE(temp_mag,3);
    calibration mag_cal = calibration_from_doubles(config.calib.mag);
    calibration grav_cal = calibration_from_doubles(config.calib.accel);
    gsl_vector_view accel = gsl_vector_view_array(cooked->accel, 3);
    gsl_vector_view mag = gsl_vector_view_array(cooked->mag, 3);
    gsl_vector_memcpy(&temp_accel, &accel.vector);
    gsl_vector_memcpy(&temp_mag, &mag.vector);
    apply_calibration(&temp_accel, &grav_cal, &accel.vector);
    apply_calibration(&temp_mag, &mag_cal, &mag.vector);
}




void sensors_read_cooked(struct COOKED_SENSORS *sensors, int count) {
    sensors_read_uncalibrated(sensors, count);
    sensors_uncalibrated_to_cooked(sensors);
}

void sensors_get_orientation(gsl_vector *orientation, int count) {
    struct COOKED_SENSORS sensors;
    gsl_vector_view mag = gsl_vector_view_array(sensors.mag, 3);
    gsl_vector_view grav = gsl_vector_view_array(sensors.accel, 3);
    sensors_read_cooked(&sensors, count);
    maths_get_orientation_as_vector(&mag.vector, &grav.vector, orientation);
}