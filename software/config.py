#!/usr/bin/python
from struct_parser import StructParser
class Config(StructParser):

    FMT = [
        ('axes', [
            ('accel', '3B'),
            ('mag',   '3B')
            ]),
        ('calib', [
            ('accel', '12f'),
            ('mag',   '12f'),
            ('laser_offset', 'i')
            ]),
        ('display_style', 'B'),
        ('length_units', 'B'),
        ('timeout', 'h')
    ]
    ALIGNMENT = 8
    
    def is_valid(self):
        return self.axes.accel[0] < 3
