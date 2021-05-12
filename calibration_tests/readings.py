import json

import numpy as np
from typing import List
import math

class Readings:
    def __init__(self, mag, grav):
        self.mag = mag
        self.grav = grav

    def __add__(self, other):
        return Readings(np.vstack((self.mag, other.mag)), np.vstack((self.grav, other.grav)))

    def __copy__(self):
        return Readings(self.mag, self.grav)

    def __len__(self):
        return len(self.mag)

    def pop(self, index):
        result = self.mag[index:index+1], self.grav[index:index+1]
        self.mag = np.vstack((self.mag[:index], self.mag[index+1:]))
        self.grav = np.vstack((self.grav[:index], self.grav[index+1:]))
        return result

    def copy(self):
        return Readings(self.mag, self.grav)


class ReadingSet:
    def __init__(self, unaligned: Readings, aligned: List[Readings]):
        self.unaligned = unaligned
        self.aligned = aligned
        self._all = sum(self.aligned, start=self.unaligned)

    @classmethod
    def create_from_file(cls, fname: str):
        if fname.endswith(".npz"):
            return cls.load_from_npz(fname)
        elif fname.endswith(".cal"):
            return cls.load_from_json(fname)
        else:
            raise ValueError("Filename must end with .npz or .cal")

    @classmethod
    def load_from_json(cls, fname):
        f = open(fname)
        g = json.load(f)

        mag = g['shots']['mag']
        grav = g['shots']['grav']

        mag = [x for x in mag if not math.isnan(x)]
        grav = [x for x in grav if not math.isnan(x)]
        m = np.reshape(mag, (-1, 3))
        g = np.reshape(grav, (-1, 3))
        return cls.create_from_readings(m, g)

    @classmethod
    def load_from_npz(cls, fname):
        data = np.load(fname)
        m = data['m'].T
        g = data['g'].T
        return cls.create_from_readings(m, g)

    @classmethod
    def create_from_readings(cls, m, g):
        unaligned = Readings(m[:8], g[:8])
        i = 8
        aligned = []
        while i < len(m):
            aligned.append(Readings(m[i:i + 8], g[i:i + 8]))
            i += 8
        return cls(unaligned, aligned)

    def all(self):
        return self._all

    def num_aligned_groups(self):
        return len(self.aligned)

    def group_length(self, group):
        return len(self.aligned[group])

    def loo_single(self, group, index):
        temp_aligned = [x.copy() for x in self.aligned]
        results = temp_aligned[group].pop(index)
        return results, ReadingSet(self.unaligned, temp_aligned), temp_aligned[group]

    def loo(self):
        return (self.loo_single(x, y) for x in range(len(self.aligned)) for y in range(len(self.aligned[x])))






