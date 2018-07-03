#define USE_AND_OR
#include "config.h"
#include "sensors.h"
#include "i2c_util.h"
#include "maths.h"
#include "mcc_generated_files/rtcc.h"

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
#define INT_EN     0x01                        //Interrupt high on data ready (0x10 for interrupt on fifo overflow)
                                            
                                               
#define USER_CTRL    0x64                      //enable FIFO and I2C and also reset it
        
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
 
	sensors_init_compass();
}

void sensors_init_compass() {
   uint8_t temp;
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
    MPU_COMMAND(0x28, SLV1_ADDR);
    MPU_COMMAND(0x29, SLV1_REG);
    MPU_COMMAND(0x2A, SLV1_CTRL);
    MPU_COMMAND(0x64, SLV1_DO);
    MPU_COMMAND(0x34, I2C_MST_DELAY);
    MPU_COMMAND(0x67, I2C_MST_DELAY_CTRL);
	MPU_COMMAND(0x37, INT_CFG);
	MPU_COMMAND(0x38, INT_EN);
        
        //enable fifo

    MPU_COMMAND(0x6A, USER_CTRL);
        
    MPU_COMMAND(0x6B, PWR_MGMT_1);
}



void byte_swap(uint16_t *word){
    *word = (*word << 8 | *word >> 8);    
}

void sensors_read_raw(struct RAW_SENSORS *sensors, bool lidar){
    int i;
    read_i2c_data(MPU_ADDRESS, 0x3B, (uint8_t *)sensors, sizeof(*sensors));
    sensors->distance = lidar ? 0 : 10; 
    for(i=0; i< 10; ++i) {
        byte_swap(&((uint16_t*)sensors)[i]);
    }
}

void sensors_read_cooked(struct COOKED_SENSORS *sensors, bool lidar) {
    struct RAW_SENSORS raw_sensors;
    int i;
    sensors_read_raw (&raw_sensors,lidar);
	sensors_raw_to_cooked(sensors,&raw_sensors);
}

void sensors_raw_to_cooked(struct COOKED_SENSORS *cooked, struct RAW_SENSORS *raw){
    // first convert to doubles with sensible units
	// also account for vagaries of sensor alignment
	cooked->accel[0] = raw->accel[AX_AXIS]*AX_POLARITY*(ACCEL_FULL_SCALE/32768.0);
	cooked->gyro[0] = raw->gyro[GX_AXIS]*GX_POLARITY*(GYRO_FULL_SCALE/32768.0);
	cooked->mag[0] = raw->mag[MX_AXIS]*MX_POLARITY*(MAG_FULL_SCALE/32768.0);

	cooked->accel[1] = raw->accel[AY_AXIS]*AY_POLARITY*(ACCEL_FULL_SCALE/32768.0);
	cooked->gyro[1] = raw->gyro[GY_AXIS]*GY_POLARITY*(GYRO_FULL_SCALE/32768.0);
	cooked->mag[1] = raw->mag[MY_AXIS]*MY_POLARITY*(MAG_FULL_SCALE/32768.0);

	cooked->accel[2] = raw->accel[AZ_AXIS]*AZ_POLARITY*(ACCEL_FULL_SCALE/32768.0);
	cooked->gyro[2] = raw->gyro[GZ_AXIS]*GZ_POLARITY*(GYRO_FULL_SCALE/32768.0);
	cooked->mag[2] = raw->mag[MZ_AXIS]*MZ_POLARITY*(MAG_FULL_SCALE/32768.0);

    cooked->temp = (raw->temp/333.87)+21.0;
    cooked->distance = (double)raw->distance/1000.0;
    //FIXME need to apply calibration here....
}


void sensors_get_orientation(struct COOKED_SENSORS *sensors, double *d) {
	double east[3];
	double north[3];
	normalise(sensors->accel);
	cross_product(sensors->accel,sensors->mag,east); // east = down x mag
	normalise(east);
	cross_product(east,sensors->accel,north);       // north= east x down
	normalise(north);
	d[0] = east[1];
	d[1] = north[1];
	d[2] = sensors->accel[1];
	normalise(d);
}

void sensors_read_leg(struct LEG *leg, double *distance) {
	struct COOKED_SENSORS sensors;
	double east[3];
	double north[3];
    struct tm datetime;
	//read curren)t date and time
    RTCC_TimeGet(&datetime);
    leg->dt = mktime(&datetime);
	sensors_read_cooked(&sensors,true);
	normalise(sensors.accel);
	cross_product(sensors.accel,sensors.mag,east); // east = down x mag
	normalise(east);
	cross_product(east,sensors.accel,north);       // north= east x down
	normalise(north);
	leg->delta[0] = east[1]*sensors.distance;
	leg->delta[1] = north[1]*sensors.distance;
	leg->delta[2] = sensors.accel[1]*sensors.distance;
	*distance = sensors.distance;
}


