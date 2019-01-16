#!/usr/bin/python

import struct
import time
import logging
import sys

class mpu9250(object):
    def __init__(self,programmer):
        self.programmer = programmer
        self.addr = 0x68
        self.dlpf_cfg = 2 #set 84HZ low pass filter
        self.fifo_lock = 0x40 #extra bytes will not be written to FIFO when full (0x00 to continue writing)
        self.ext_sync_set = 0
        self.sample_rate = 20
        self.gyro_full_scale = 1000 # pick from 250,500,1000,2000 degree/second
        self.accel_full_scale = 2 # pick from 2,4,8,16g
        self.accel_hpf = 0 # high pass filter. 0 is inactive. 
                           # 1:5Hz, 2:2.5Hz, 3:1.25Hz, 5:0.68Hz, 
                           # 6: differential from previous reading
        self.fifo_sensors = 0x79 # 0x80 = temp sensor
                                 # 0x40,0x20,0x10 = gyros xyz respectively
                                 # 0x08 = accelerometer
                                 # 0x04,0x02,x01 = slaves 2,1,0 respectively
                                 
        self.int_cfg = 0x20 #active high(0), push-pull(0), latched(1),cleared on readof status(0),
                            #FSYNC interrupt disables(00),i2c bypass disabled(0), clock output disabled(0)
        self.user_ctrl = 0x64 #enable FIFO and I2C and also reset it
        
        self.pwr_mgmt_1 = 0x01#PLL with X-axis gyro as clock. 0x08 disables temp sensor
        
        #I2C stuff
        self.i2c_mst_ctrl = 0x40
        self.slv0_addr = 0x8C
        self.slv0_reg = 0x03
        self.slv0_ctrl = 0xD6
        self.slv1_addr = 0x0C
        self.slv1_reg = 0x0A
        self.slv1_ctrl = 0x81
        self.slv1_do = 0x11
        self.i2c_mst_delay_ctrl = 0x83
        self.i2c_mst_delay = 0x01
    
    #general purpose comms functions    
    def read_byte(self,register):
        return self.programmer.read_i2c_address(self.addr,register,1)[0]
    
    def read_block(self,register,length):
        data = self.programmer.read_i2c_address(self.addr,register,length)
        #pack it all into a struct
        return struct.pack('%dB' % len(data),*data)    
    
    def read_word(self,register):
        val = self.read_block(register,2)
        return struct.unpack('>H',val)[0]
        
    def write_byte(self,register,value):
        logging.debug("writing %02x with %02x" % (register,value))
        self.programmer.write_i2c(self.addr,[register,value])
        
            
    def configure(self):
        #reset FIFO, I2C, signal conditioning...
        self.write_byte(0x6A,0)
        self.write_byte(0x6A,7)

        if self.dlpf_cfg in (0,7):
            clock = 8000
        else:
            clock = 1000
        smplrt_div = (clock/self.sample_rate)-1
        self.write_byte(0x19,int(smplrt_div))
        self.write_byte(0x1A,self.fifo_lock + self.dlpf_cfg+self.ext_sync_set*8)
        
        gyro_fs_map = {250:0,500:1,1000:2,2000:3}
        if self.gyro_full_scale in gyro_fs_map:
            self.write_byte(0x1B,gyro_fs_map[self.gyro_full_scale]*8)
        else:
            self.write_byte(0x1B,3*8)
        
        #set accel full scale and high_pass_filter
        accel_fs_map = {2:0,4:1,8:2,16:3}
        if self.accel_full_scale in accel_fs_map:
            self.write_byte(0x1C,accel_fs_map[self.accel_full_scale]*8+self.accel_hpf)
        else:
            self.write_byte(0x1C,3*8+self.accel_hpf)
            
        #set fifo enablement
        self.write_byte(0x23,self.fifo_sensors)            
        
        #put i2c control stuff here
        self.write_byte(0x24, self.i2c_mst_ctrl)
        self.write_byte(0x25, self.slv0_addr)
        self.write_byte(0x26, self.slv0_reg)
        self.write_byte(0x27, self.slv0_ctrl)
        self.write_byte(0x28, self.slv1_addr)
        self.write_byte(0x29, self.slv1_reg)
        self.write_byte(0x2A, self.slv1_ctrl)
        
        self.write_byte(0x64, self.slv1_do)
        self.write_byte(0x34, self.i2c_mst_delay)
        self.write_byte(0x67, self.i2c_mst_delay_ctrl)
        
        #enable fifo

        self.write_byte(0x6A,self.user_ctrl)
        
        self.write_byte(0x6B,self.pwr_mgmt_1)
        
    def read_fifo(self,sensor_count):
        results = []
        size = self.read_word(0x72)
        logging.debug("fifo reads %d bytes" % size)
        while(size>sensor_count*2):
            results.append(self.read_block(0x74,sensor_count*2))
            size -= sensor_count*2
        return results
        
    def read_sensors(self):
        results = self.read_block(0x3B,20)
        v = struct.unpack('>10h',results)
        sensors = {'accel':[v[0],v[1],v[2]],
                   'temp':v[3],
                   'gyro':[v[4],v[5],v[6]],
                   'compass':[v[7],v[8],v[9]]}
        return sensors,v
        
    def enable_fifo(self,enable=True):
        if enable:
            self.write_byte(0x23,self.fifo_sensors)
            self.write_byte(0x6A,self.user_ctl | 0x44)
        else:
            self.write_byte(0x23,0)
            self.write_byte(0x6A,self.user_ctl & 0b10111011)
def main():
    #logging.basicConfig(level=logging.DEBUG)
    import bootloader
    print "Connecting to device"
    prog = bootloader.Programmer()
    print "Device found"
    mpu = mpu9250(prog)
    mpu.configure()
    output = ""
    print("aX\taY\taZ\ttemp\tgX\tgY\tgZ\tmX\tmY\tmZ")
    while True:
        time.sleep(0.3)
        sensors,raw_vals = mpu.read_sensors()
        print "\t".join("%5d" % i for i in raw_vals),'\r',
        sys.stdout.flush()
#        for readings in mpu.read_fifo(9):
#            output += "\t".join("%05d" % i for i in struct.unpack('>9h',readings))
#            output += "\n"
#    print(output)
if __name__=="__main__":
    main()
