#!/usr/bin/env python
import re
import datetime
import sys
import math
import time
import random
import struct

dt = None

tau = math.pi/180.0

number = r"([+-]?\d+(?:\.\d+)?)"
leg_pattern = r"(\d+)\s+(\d+)\s+" + number +r"\s+" + number +r"\s+" +number
output = open("output.dat","w")
for survey, fname in enumerate(sys.argv[1:]):
    for line in open(fname):
        if line.startswith("*date"):
            dt = datetime.datetime.strptime(line[6:16],"%Y.%m.%d")
        if line.startswith("*data passage"):
            break
        match = re.match(leg_pattern, line)
        if match:
            frm, to, tape, compass, clino =  match.groups()
            tape = float(tape)
            compass = float(compass)
            clino = float(clino)
            ext = math.cos(clino*tau)*tape
            dz = math.sin(clino*tau)*tape
            dy = math.cos(compass*tau)*ext
            dx = math.sin(compass*tau)*ext
            tm = time.mktime(dt.timetuple())
            print tm, survey, frm, to, dx, dy, dz
            dt += datetime.timedelta(seconds=random.randint(0,300))
            output.write(struct.pack("IHBB3f",tm, survey, int(frm), int(to), dx, dy, dz))
