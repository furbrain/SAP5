#!/usr/bin/env python3
import numpy as np
import json
import sys
import math
from pathlib import Path

fname = Path(sys.argv[1])
f = open(fname)
g = json.load(f)

mag = g['shots']['mag']
grav = g['shots']['grav']

mag = [x for x in mag if not math.isnan(x)]
grav = [x for x in grav if not math.isnan(x)]
m = np.reshape(mag,(-1,3))
g = np.reshape(grav,(-1,3))
print(m)
print(g)
np.savez(fname.with_suffix(".npz"),m=m.T, g=g.T)
