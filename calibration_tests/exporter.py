#!/usr/bin/python3

import numpy as np
import sys

data = np.load(sys.argv[1]) 

m = data['m'].reshape((-1,3))
g = data['g'].reshape((-1,3))
print("double mag_sample_data[] = {")
for row in m:
	print(', '.join(str(x) for x in row)+',')
print("};\n")
print("double grav_sample_data[] = {")
for row in g:
	print(', '.join(str(x) for x in row)+',')
print("};")
