#!/usr/bin/python
from struct_parser import StructParser
import numpy as np
import scipy.stats

import bootloader
p = bootloader.Programmer()

def normalise(vectors):
    return vectors/np.linalg.norm(vectors, axis=0)

class Leg():
    __FMT__ = [
        ('tm', 'I'),
        ('survey', 'H'),
        ('from', 'B'),
        ('to', 'B'),
        ('delta', '3f')
    ]



raw = p.read_program(0x9d009800, 5600)
data1 = np.array([Leg(raw[i*24:(i+1)*24]).delta for i in range(100)])
print "-----------------"
data2 = np.array([Leg(raw[i*24:(i+1)*24]).delta for i in range(100,200)])


print np.mean(data1, axis=0)
print np.std(data1, axis=0)
print np.mean(data2, axis=0)
print np.std(data2, axis=0)
print scipy.stats.ttest_ind(data1,data2, axis=0)
print scipy.stats.levene(data1[:,0], data2[:,0])
print scipy.stats.levene(data1[:,1], data2[:,1])
print scipy.stats.levene(data1[:,2], data2[:,2])
a = np.mean(normalise(data1.T).T, axis=0)
b = np.mean(normalise(data2.T).T, axis=0)
print a, b, a-b
print np.linalg.norm(a-b)
print np.rad2deg(np.linalg.norm(a-b))
