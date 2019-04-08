#!/usr/bin/env python3

# location of usb hole...
# x centre offset = +0.8mm (to the right)
# z offset = 6 + 1.6 + 2mm = 9.6mm

# 3 layers - clear a space for the whole assembly to fit - depth 0.3 mm 11x5mm square
# 2nd layer - space for o-ring 0.5mm all round.. = 9.7 x 4.6, 0.5 deep
# 1st layer - main hole, extra 8.7x3.6 rounded

import gcodelib as gc

SAFE_HEIGHT=1.0
RAPID = 200
SLOW = 100


##TODO## add mounting hole for lanyard - 3.1mmdiameter hole and 4mm shoulder?
def rounded_hole(x, y, z, tool_radius):
    """returns a hole with semicircular ends on x-axis (x > y)"""
    x -= tool_radius*2
    y -= tool_radius*2
    r = y/2
    x -= y
    result = gc.Aggregate()
    #go to top left
    result.add(gc.Rapid(-x/2,y/2,SAFE_HEIGHT, f=RAPID))
    result.add(gc.Line(-x/2,y/2,z, f=SLOW))
    result.add(gc.Line(x/2,y/2))
    result.add(gc.Arc(x/2+r, 0, r=r))
    result.add(gc.Arc(x/2,-y/2, r=r))
    result.add(gc.Line(-x/2,-y/2))
    result.add(gc.Arc(-x/2-r, 0, r=r))
    result.add(gc.Arc(-x/2, y/2, r=r))
    result.add(gc.Rapid(z=SAFE_HEIGHT))
    return result
    
def rectangle(x,y,z, tool_radius):    
    x -= tool_radius*2
    y -= tool_radius*2
    result = gc.Aggregate()
    result.add(gc.Rapid(-x/2,y/2,SAFE_HEIGHT, f=RAPID))
    result.add(gc.Line(-x/2,y/2,z, f=SLOW))
    result.add(gc.Line(x/2,y/2))
    result.add(gc.Line(x/2,-y/2))
    result.add(gc.Line(-x/2,-y/2))
    result.add(gc.Line(-x/2,y/2))
    result.add(gc.Rapid(z=SAFE_HEIGHT))
    return result

ensemble = gc.Aggregate(
    rounded_hole(8.7, 3.6, -2.0, 0.75),
    rounded_hole(8.7, 3.6, -4.0, 0.75),
    rounded_hole(9.7, 4.6, -1.4, 0.75),
    rectangle(11, 5, -0.8, 0.75))

print ("M03 S750\n")
print(gc.Translate([-0.8,10.5,0], ensemble))
print("M05\n")
