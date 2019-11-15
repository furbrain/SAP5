#!/usr/bin/python3
from struct_parser import StructParser
class BatteryRecord(StructParser):

    FMT = [
        ('count', 'I'),
        ('voltage', 'f'),
        ]
    ALIGNMENT = 8
    
    def is_valid(self):
        return self.count < 0xffff
        
if __name__ == "__main__":
	import bootloader
	APP_CONFIG_LOCATION = 0x9D009000
	APP_LEG_LOCATION = 0x9D009800
	APP_CONFIG_SIZE = 0x00000800
	APP_LEG_SIZE = 0x00002800
	
	prog = bootloader.Programmer()
	i = 0
	print("start")
	while True:
		text = prog.read_program(APP_LEG_LOCATION+i*8, 8)
		conf = BatteryRecord.from_buffer(text)	
		if not conf.is_valid():
		    break
		i += 1
		print(conf.count, ",", conf.voltage)

