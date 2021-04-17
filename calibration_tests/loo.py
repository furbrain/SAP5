#!/usr/bin/env python3
import sys

import numpy as np

from calset import CalSet, graph
import readings

if len(sys.argv) > 1:
    readingset = readings.load_from_npz(sys.argv[1])
else:
    readingset = readings.load_from_npz("data.npz")


uniformity = []
accuracy = []
combined = []
orders = list(range(10))
for order in orders:
    print("Processing order: ", order)
    uniformity.append(0)
    accuracy.append(0)
    count = 0
    for (mag, grav), rset, group in readingset.loo():
        c = CalSet.full_calibration(rset, order=order)
        uniformity[-1] += c.check_uniformity(mag, grav)
        accuracy[-1] += c.check_alignment_single(mag, grav, group)
        count += 1
uniformity = np.array(uniformity) / count
accuracy = np.array(accuracy) / count
combined = uniformity+accuracy
graph(orders, accuracy, uniformity, combined, labels=["accuracy", "uniformity", "combined"])
