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

GSL_MATRIX_DECLARE(temp_mag_readings, SAMPLES_PER_READING, 3);
GSL_MATRIX_DECLARE(temp_grav_readings, SAMPLES_PER_READING, 3);


#define MPU_ADDRESS 0x68
#define MPU_COMMAND(command,data) write_i2c_data2(MPU_ADDRESS,command,data)

/* MPU configuration macros */
#define DLPF_CFG 2                             //set 84HZ low pass filter
#define SAMPLE_RATE 100
#if DLPF_CFG==7 || DLPF_CFG==0
    #define CLOCK 8000
#else
    #define CLOCK 1000
#endif        
#define SMPLRT_DIV  (CLOCK/SAMPLE_RATE)-1
#define FIFO_LOCK    0x40                      //extra bytes will not be written to FIFO when full (0x00 to continue writing)
#define EXT_SYNC_SET 0
#define GYRO_FULL_SCALE 250                   // pick from 250,500,1000,2000 degree/second
#define ACCEL_FULL_SCALE 2                     // pick from 2,4,8,16g
#define MAG_FULL_SCALE 4912                    // 4912 uT full-scale magnetic range
#define ACCEL_HPF 0                            // high pass filter. 0 is inactive. 
                                               // 1:5Hz, 2:2.5Hz, 3:1.25Hz, 5:0.68Hz, 
                                               // 6: differential from previous reading
#define FIFO_SENSORS    0x79                   // 0x80    temp sensor
                                               // 0x40,0x20,0x10    gyros xyz respectively
                                               // 0x08    accelerometer
                                               // 0x04,0x02,x01    slaves 2,1,0 respectively
                                 
#define INT_CFG    0x30                        //active high(0), push-pull(0), latched(1),cleared on any read(1),
                                               //FSYNC interrupt disables(00),i2c bypass disabled(0), clock output disabled(0)
#define INT_EN     0x00                        //Interrupt high on data ready (0x10 for interrupt on fifo overflow)
                                            
                                               
#define USER_CTRL    0x67                      //enable FIFO and I2C and also reset it
        
#define PWR_MGMT_1    0x0                      //PLL with X-axis gyro as clock. 0x08 disables temp sensor
        
//I2C stuff
#define I2C_MST_CTRL    0x40
#define SLV0_ADDR    0x8C
#define SLV0_REG    0x03
#define SLV0_CTRL    0xD7
#define SLV1_ADDR    0x0C
#define SLV1_REG    0x0A
#define SLV1_CTRL    0x81
#define SLV1_DO    0x11
#define I2C_MST_DELAY_CTRL    0x83
#define I2C_MST_DELAY    0x00



void sensors_init() {
    //reset FIFO, I2C, signal conditioning...
    uint8_t temp;
    wdt_clear();
    MPU_COMMAND(0x6A,0);
    MPU_COMMAND(0x6A,7);

    MPU_COMMAND(0x19,(uint8_t)(SMPLRT_DIV));
    MPU_COMMAND(0x1A,FIFO_LOCK | DLPF_CFG | (EXT_SYNC_SET*8));
    switch (GYRO_FULL_SCALE){
        case 250:
            temp=0;
            break;
        case 500:
            temp=1;
            break;
        case 1000:
            temp=2;
            break;
        case 2000:
        default:
            temp=3;
            
            break;
        }
    wdt_clear();
    MPU_COMMAND(0x1B,temp<<3);
    //set accel full scale and high_pass_filter
    switch (ACCEL_FULL_SCALE){
        case 2:
            temp=0;
            break;
        case 4:
            temp=8;
            break;
        case 8:
            temp=16;
            break;
        case 16:
        default:
            temp=24;
            break;
        }
    MPU_COMMAND(0x1C,temp);
	MPU_COMMAND(0x1D,DLPF_CFG);
    //set fifo enablement
    MPU_COMMAND(0x23, FIFO_SENSORS);            
        
        //put i2c control stuff here
    MPU_COMMAND(0x24, I2C_MST_CTRL);
    MPU_COMMAND(0x25, SLV0_ADDR);
    MPU_COMMAND(0x26, SLV0_REG);
    MPU_COMMAND(0x27, SLV0_CTRL);
    wdt_clear();
    MPU_COMMAND(0x28, SLV1_ADDR);
    MPU_COMMAND(0x29, SLV1_REG);
    MPU_COMMAND(0x2A, SLV1_CTRL);
    MPU_COMMAND(0x64, SLV1_DO);
    MPU_COMMAND(0x34, I2C_MST_DELAY);
    MPU_COMMAND(0x67, I2C_MST_DELAY_CTRL);
	MPU_COMMAND(0x37, INT_CFG);
	MPU_COMMAND(0x38, INT_EN);
    wdt_clear();
    //enable fifo
    MPU_COMMAND(0x6A, USER_CTRL);
    MPU_COMMAND(0x6B, PWR_MGMT_1);
    wdt_clear();
}



void byte_swap(uint16_t *word){
    *word = (*word << 8 | *word >> 8);    
}

void sensors_read_raw(struct RAW_SENSORS *sensors){
    int i;
    if (read_i2c_data(MPU_ADDRESS, 0x3B, (uint8_t *)sensors, sizeof(*sensors))) {
        THROW_WITH_REASON("I2C communication failed", ERROR_MAGNETOMETER_FAILED);
    }
    for(i=0; i< 10; ++i) {
        byte_swap(&((uint16_t*)sensors)[i]);
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
        cooked->mag[i] = (raw->mag[i]/32768.0) * MAG_FULL_SCALE/2;
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







