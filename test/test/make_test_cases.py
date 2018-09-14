#!/usr/bin/python

import numpy as np
import re


def bracketiser(arr):
    arr = str(arr).replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('  ',' ')
    arr = arr.replace('[ ','[')    
    arr = arr.replace(' ]',']')    
    arr = arr.replace('[','{')
    arr = arr.replace(']]','}]')
    arr = arr.replace(']','},')
    arr = re.sub('([0123456789\.]) ','\\1,', arr)
    arr = arr.replace('\n','')
    arr = arr.replace(',}','}')
    arr = re.sub(',$','',arr)
    return arr

np.set_printoptions(suppress=True, precision=4)
np.random.seed(10)
for j in range(2):
    calib = np.identity(4)
    for i in range(5):
        delta = (np.random.random((4,4))*2)-1
        delta[3][0:3] = 0
        delta[3][3] = 1
        calib = np.matmul(calib, delta)
        print "{%s, %s}," % (bracketiser(delta[0:3]), bracketiser(calib[0:3]))
