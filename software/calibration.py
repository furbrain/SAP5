#!/usr/bin/python3
import datetime
import math

from struct_parser import StructParser

    
class Calibration(StructParser):
    FMT = [
        ('mag', '256f'),
        ('grav', '256f'),
    ]
    ALIGNMENT = 8
    
    def is_valid(self):
        return not math.isnan(self.mag[0])

def read_cal(bootloader):
    APP_CAL_LOCATION = 0x9D009800
    APP_CAL_SIZE = 0x00002800
    data = bootloader.read_program(APP_CAL_LOCATION, Calibration.get_len())
    cal = Calibration.from_buffer(data)
    return cal
