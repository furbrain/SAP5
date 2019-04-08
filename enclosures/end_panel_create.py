#!/usr/bin/env python3

# location of usb hole...
# x centre offset = +0.8mm (to the right)
# z offset = 6 + 1.6 + 2mm = 9.6mm

# 3 layers - clear a space for the whole assembly to fit - depth 0.3 mm 11x5mm square
# 2nd layer - space for o-ring 0.5mm all round.. = 9.7 x 4.6, 0.5 deep
# 1st layer - main hole, extra 8.7x3.6 rounded

import gcodelib as gc

SAFE_HEIGHT=1.0
RAPID = 400
SLOW = 200

TOOL = 0.75
INCREMENT = 1.0

OUTER = (59.6, 22.6, 3.6)
INNER = (56.9, 20.6, 1.3) 


def rectangle(x, y, depth):
    x += TOOL/2
    y += TOOL/2
    z = 0
    result = gc.Aggregate()
    result.add(gc.Rapid(-x/2, -y/2, SAFE_HEIGHT, f=RAPID))
    while z > -depth:
        z= max(z-INCREMENT, -depth)
        result.add(gc.Line(z=z, f=SLOW))
        result.add(gc.Line(x=x/2))
        result.add(gc.Line(y=y/2))
        result.add(gc.Line(x=-x/2))
        result.add(gc.Line(y=-y/2))
    result.add(gc.Line(z=SAFE_HEIGHT))
    return result

def drill_hole(radius, depth):
    cut_radius = radius-TOOL
    hole = gc.Aggregate()
    hole.add(gc.Rapid(-cut_radius, 0, SAFE_HEIGHT, f=RAPID)) #go to start point
    hole.add(gc.Line(z=0, f=SLOW)) # go to surface
    i=0
    while (i> -depth):
        i= max(i-INCREMENT, -depth)
        hole.add(gc.Arc(z=i, i=cut_radius))
    hole.add(gc.Arc(z=i, i=cut_radius))
    hole.add(gc.Rapid(z=SAFE_HEIGHT))
    return hole






ensemble = gc.Aggregate(
    gc.Translate([+0.8,15.5-OUTER[1]/2,0],
        drill_hole(2.3,1.3),
        drill_hole(1.6,3.6)),
    rectangle(*INNER),
    rectangle(*OUTER))

print ("M03 S750\n")
print(gc.Translate([OUTER[0]/2,OUTER[1]/2,0], ensemble))
print("M05\n")
