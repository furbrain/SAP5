#!/usr/bin/python
import struct
import numpy as np
import scipy.stats
from collections import namedtuple

import bootloader
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

p = bootloader.Programmer()

def normalise(vectors):
    return vectors/np.linalg.norm(vectors, axis=0)

Axes = namedtuple('Axes','accel mag')
Axis = namedtuple('Axis','x y z')
Calib = namedtuple('Calib', 'accel mag laser_offset')

class Leg():
    def __init__(self, data):
        mx, my, mz = struct.unpack_from("3f", data)
        self.a = np.array([mx, my, mz])
        #print self.a
        
    def __str__(self):
        return str(self.delta)
        
    @staticmethod
    def get_all():
        raw = p.read_program(0x9d009800, 5800)
        return np.array([Leg(raw[i*12:(i+1)*12]).a for i in range(400)])

class Config():
    def __init__(self, data):
        self.axes = Axes(Axis(*struct.unpack_from("3c",data[0:3])),
                         Axis(*struct.unpack_from("3c",data[3:6])))
        self.calib = Calib(np.reshape(struct.unpack_from("12f",data[8:56]),(4,3)),
                           np.reshape(struct.unpack_from("12f",data[56:104]),(4,3)),
                           struct.unpack_from("i",data[102:106])[0])
#    struct {
#        matrixx accel;
#        matrixx mag;
#        accum laser_offset;
#    } calib;
#    uint8_t display_style;
#    uint8_t length_units;
#    uint16_t timeout;
#};
    @staticmethod
    def get_latest():
        raw = p.read_program(0x9d009000,0x800)
        return Config(raw)

data1 = Leg.get_all()
config = Config.get_latest()
print config.axes.accel
print config.axes.mag
print config.calib.mag
print config.calib.accel
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

#ax.scatter(data1[:240,0], data1[:240,1], data1[:240,2])
#ax.scatter(data1[240:,0], data1[240:,1], data1[240:,2])
#plt.show()
