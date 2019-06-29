#!/usr/bin/python
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
        
if __name__ == "__main__":
	import bootloader
	APP_CONFIG_LOCATION = 0x9D009000
	APP_LEG_LOCATION = 0x9D009800
	APP_CONFIG_SIZE = 0x00000800
	APP_LEG_SIZE = 0x00002800
	
	prog = bootloader.Programmer()
	i = 0
	while True:
		text = prog.read_program(APP_CONFIG_LOCATION+i*Config.get_len(), Config.get_len())
		if text[0]==0xff:
			break
		conf = Config.from_buffer(text)	
		i += 1
		print conf

