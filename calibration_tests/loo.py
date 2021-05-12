#!/usr/bin/env python3
import json
import os.path
import sys

import numpy as np

from calset import CalSet, graph
import readings

MAX_ORDERS = 7
LOO = True

if len(sys.argv) > 1:
    fnames = sys.argv[1:]
else:
    fnames = ["data.npz"]

all_reading_sets = [readings.ReadingSet.create_from_file(f) for f in fnames]
all_data = {
    "uniformity": {},
    "accuracy": {},
    "combined": {}
}

for readingset, fname in zip(all_reading_sets, (os.path.splitext(f)[0] for f in fnames)):
    print(f"Processing {fname}")
    uniformity = []
    accuracy = []
    combined = []
    orders = list(range(MAX_ORDERS))
    if LOO:
        for order in orders:
            print("Processing order: ", order)
            uniformity.append(0)
            accuracy.append(0)
            count = 0
            for (mag, grav), rset, group in readingset.loo():
                c = CalSet.full_calibration(rset, order=order)
                uniformity[-1] += c.check_uniformity(mag, grav)
                accuracy[-1] += c.check_alignment_single(mag, grav, group)
                print("LOO: ", c.check_alignment_single(mag, grav, group))
                count += 1
        print("ORDERED", uniformity, accuracy)
        uniformity = np.array(uniformity) / count
        accuracy = np.array(accuracy) / count
        print("NORMALISED", uniformity, accuracy)
    else:
        for order in orders:
            print ("Processing order: ", order)
            c = CalSet.full_calibration(readingset, order)
            uniformity.append(c.check_uniformity_multiple(readingset))
            accuracy.append(c.check_alignment2(readingset) )
    combined = uniformity + accuracy
    all_data["accuracy"][fname] = accuracy
    all_data["uniformity"][fname] = uniformity
    all_data["combined"][fname] = combined
    print(f"Uniformity for {fname}: {uniformity}")
    print(f"Accuracy for {fname}: {accuracy}")
    #graph(orders, accuracy, uniformity, combined, labels=["accuracy", "uniformity", "combined"])

class NumpyEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return json.JSONEncoder.default(self, obj)

with open("results.json","w") as results:
    json.dump(all_data,results,cls=NumpyEncoder)