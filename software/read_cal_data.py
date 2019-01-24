#!/usr/bin/python
import struct
from struct_parser import StructParser
import numpy as np
import scipy.stats
from collections import namedtuple

import bootloader
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

p = bootloader.Programmer()

def normalise(vectors):
    return vectors/np.linalg.norm(vectors, axis=0)


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

class Config(StructParser):
    __FMT__ = [
        ('axes', [
            ('accel', '3c'),
            ('mag',   '3c')
            ]),
        ('calib', [
            ('accel', '12f'),
            ('mag',   '12f'),
            ('laser_offset', 'i')
            ]),
        ('display_style', 'B'),
        ('length_units', 'B'),
        ('timeout', 'h')
    ]

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
