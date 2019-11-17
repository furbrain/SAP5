#!/usr/bin/python3
from struct_parser import StructParser
class Config(StructParser):

    FMT = [
        ('axes', [
            ('accel', '3B'),
            ('mag',   '3B')
            ]),
        ('dummy', 'h'),
        ('calib', [
            ('accel', '12f'),
            ('mag',   '12f'),
            ('laser_offset', 'f')
            ]),
        ('display_style', 'B'),
        ('length_units', 'B'),
        ('timeout', 'h')
    ]
    ALIGNMENT = 8
    
    def is_valid(self):
        return self.axes.accel[0] < 3
        
def get_config(bootloader):
    APP_CONFIG_LOCATION = 0x9D009000
    APP_CONFIG_SIZE = 0x00000800
    #FIXME provide default config in case not yet been saved to flash
    for i in range(APP_CONFIG_LOCATION, APP_CONFIG_LOCATION+APP_CONFIG_SIZE, Config.get_len()):
	    text = bootloader.read_program(i, Config.get_len())
	    if text[0]==0xff:
		    break
	    conf = Config.from_buffer(text)	
    return conf

