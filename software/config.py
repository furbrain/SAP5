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
class Config(StructParser):

    FMT = [
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
    ALIGNMENT = 8
    
    def is_valid(self):
        return self.axes.accel[0] < 3
    
    @staticmethod
    def get_latest():
        raw = p.read_program(0x9d009000,0x800)
        return Config(raw)


