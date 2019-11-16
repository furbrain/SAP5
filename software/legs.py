#!/usr/bin/python3
import numpy as np
import os
import collections
import datetime

from struct_parser import StructParser
import bootloader

def normalise(vectors):
    return vectors/np.linalg.norm(vectors, axis=0)
    
    

def get_surveys(leg_list):
    surveys = {}
    survey_nums  = {x.survey for x in leg_list}
    for s in survey_nums:
        surveys[s] = {'legs': [x for x in leg_list if x.survey==s]}   
    for s, dct in surveys.items():
        dct['survey'] = s
        dct['time'] = datetime.datetime.fromtimestamp(min(x.time for x in dct['legs']))
        all_stations = {x.frm for x in dct['legs']} 
        all_stations.update(x.to for x in dct['legs'])
        dct['station_count'] = len(all_stations)
        dct['leg_count'] = len(dct['legs'])
    return surveys    
    
class Leg(StructParser):
    FMT = [
        ('time', 'I'),
        ('survey', 'H'),
        ('frm', 'B'),
        ('to', 'B'),
        ('delta', '3f')
    ]
    ALIGNMENT = 8
    
    def is_valid(self):
        return self.time != 0xffffffff

def read_legs():
    if "PONY_DEBUG" in os.environ:
        with open("test_legs.dat", "rb") as f:
            data = f.read()
            return Leg.read_array(data)


if __name__ == "__main__":
    surveys = get_surveys(read_legs())
    for s in surveys.values():
        print("{survey}: {time} {station_count} {leg_count}".format(**s))
