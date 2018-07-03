#define USE_AND_OR
#include "config.h"
#include <libpic30.h>
#include <incap.h>
#include <ports.h>
#include <PPS.h>
#include <timer.h>
#include <Rtcc.h>
#include "sensors.h"
#include "i2c_util.h"
#include "peripherals.h"

/*LIDAR defines */
#define LIDAR_ADDRESS 0x62
#define LIDAR_COMMAND(command,data) write_i2c_data2(LIDAR_ADDRESS,command,data,I2C_STANDARD)
#define LIDAR_READ(command,data,length) read_i2c_data(LIDAR_ADDRESS,command,data,length,I2C_STANDARD)

#define MPU_ADDRESS 0x68
#define MPU_COMMAND(command,data) write_i2c_data2(MPU_ADDRESS,command,data,I2C_FAST)

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
 
    #ifdef LIDAR_TESTING
    /* connect IC3 to detect the rising edge of the LIDAR pulse */
	//iPPSInput(IN_FN_PPS_IC3,IN_PIN_PPS_RP4);
	/* connect IC12 to detect the falling edge */
	iPPSInput(IN_FN_PPS_IC1,IN_PIN_PPS_RP4);

    #else
    #endif
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
    read_i2c_data(MPU_ADDRESS, 0x3B, (uint8_t *)sensors, sizeof(*sensors),I2C_FAST);
    sensors->distance = lidar ? sensors_read_lidar() : 10; 
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


#ifdef LIDAR_TESTING
volatile int lidar_average_count = 32;
volatile uint32_t lidar_accumulator;
volatile uint32_t lidar_last_reading;
volatile int lidar_accumulator_count;
volatile uint32_t last_start;

uint32_t sensors_read_lidar(){
    //FIXME put lidar calibration code here
    uint32_t latest_lidar = lidar_last_reading;
    latest_lidar /= lidar_average_count;
    latest_lidar /= 16; //gives result in mm
#ifdef DEBUG
    if (latest_lidar==0) latest_lidar = 10000; //return 10m if lidar inoperative
#endif
    return latest_lidar;
}


void __attribute__ ((interrupt,no_auto_psv,)) _IC1Interrupt() {
    uint32_t reading;
    int i=0;
    IC1_Clear_Intr_Status_Bit;
    reading = (uint32_t)IC1BUF + ((uint32_t)IC2BUF << 16);
    if (PORTBbits.RB4) {
	    last_start = reading;
    } else {
	lidar_accumulator += (reading - last_start);
	lidar_accumulator_count++;
	if (lidar_accumulator_count>=lidar_average_count) {
		lidar_accumulator_count=0;
		lidar_last_reading = lidar_accumulator;
		lidar_accumulator=0;
		LATBbits.LATB1 = !LATBbits.LATB1;
	}
    }
}

void sensors_enable_lidar(bool on) {
    if (on) {
		LAT_LIDAR_ENABLE = 1;
		lidar_accumulator_count=0;
		lidar_last_reading = lidar_accumulator = 0;
		OpenCapture12(IC_IDLE_CON | IC_SYSCLK_SRC | IC_EVERY_EDGE | IC_INT_1CAPTURE,
		      IC_CASCADE_ENABLE | IC_SYNC_ENABLE | IC_SYNC_TRIG_IN_DISABLE);
	//IC2CON2bits.ICTRIG=0;
        ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_2);
        EnableIntIC1;
    } else {
        DisableIntIC1;
        CloseCapture12();
		LAT_LIDAR_ENABLE = 0;
    }
}
#else
int32_t sensors_read_lidar(){
	int error = 0;
	int count;
	uint16_t dist_hi = 0;
	uint8_t dist_lo = 0;
	LIDAR_COMMAND(1,0);
	delay_ms(15);
	for(count=0;count<30;count++) {
		if (LIDAR_COMMAND(0,4)==0) break;
		delay_ms(2);
	}
	if (count>=30) return -32;
 	for(count=0;count<30;count++) {
 		error = LIDAR_READ(0x01,(uint8_t*) &dist_lo,1);
 		if (error==0) break;
 		delay_ms(2);
 	}
 	if (count>=30) return -16;
	if (dist_lo!=6) {
		if ((dist_lo & 1)==0) return -dist_lo;
		if (dist_lo & 0x88) return -dist_lo;
	}
	for(count=0;count<30;count++) {
		error = LIDAR_READ(0x8f,(uint8_t*) &dist_hi,2);
		if (error==0) break;
		delay_ms(2);
	}
	if (count>=30) return error;
	byte_swap(&dist_hi);
	return dist_hi;
}

void sensors_enable_lidar(bool on) {
    if (on) {
		LAT_LIDAR_ENABLE = 1;
		//wait for LIDAR to startup
			__delay_ms(20);
		//send I2C initialisation
			LIDAR_COMMAND(0,0);
			LIDAR_COMMAND(0x65,0x0E);
			LIDAR_COMMAND(0x04,0x0D);
			//__delay_ms(10);
			//LIDAR_COMMAND(2,128); /* high accuracy */

	} else {
		LAT_LIDAR_ENABLE = 0;
	}
}

#endif

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
	rtccTimeDate datetime;
	//read current date and time
	RtccReadTimeDate(&datetime);
	leg->dt.year = datetime.f.year;
	leg->dt.month = datetime.f.mon;
	leg->dt.day = datetime.f.mday;
	leg->dt.hour = datetime.f.hour;
	leg->dt.minute = datetime.f.min;
	leg->dt.second = datetime.f.sec;
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


