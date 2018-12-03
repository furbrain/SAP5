#!/usr/bin/python
import struct
import numpy as np
import scipy.stats

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
        print self.a
        
    def __str__(self):
        return str(self.delta)

raw = p.read_program(0x9d009800, 5800)
data1 = np.array([Leg(raw[i*12:(i+1)*12]).a for i in range(240)])
print "-----------------"
data2 = np.array([Leg(raw[i*12:(i+1)*12]).a for i in range(240,480)])

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

ax.scatter(data1[:,0],data1[:,1],data1[:,2])
#ax.scatter(data2[:,0],data2[:,1],data2[:,2])
plt.show()
