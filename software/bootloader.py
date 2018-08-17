#!/usr/bin/python

import sys
import re
import operator
import usb
import struct
import datetime
import time

import sparse_list
# plans...
# load in hex file - possibly munged to separate out the flash bits etc...
# probably best do this directly, and load into program, flash and config mems.


CLEAR_FLASH = 100
SEND_DATA = 101
GET_CHIP_INFO = 102
REQUEST_DATA = 103
SEND_RESET = 105

# I2C commands 
WRITE_I2C_DATA = 110
READ_I2C_DATA = 111
CHECK_I2C_READY = 112
WRITE_DISPLAY = 113
READ_EEPROM_DATA = 114

# RTCC commands
WRITE_DATETIME = 120
READ_DATETIME = 121

# UART commands
WRITE_UART = 130
READ_UART = 131

def clean_list(buffer):
    for i in range(0,len(buffer)):
        if buffer[i] is None:
            buffer[i] = 0x00

class HexFileError(Exception):
        def _init__(self,value):
            self.value = value
        def __str__(self):
            return repr(self.value)

class HexFile:
    def __init__(self,fname):
        self.program = sparse_list.SparseList(0x100000000)
        page = 0;
        f = file(fname,"rU")
        for s in f:
            if (s[0] == ":"):
                #good - it starts sensibly...
                bytes = s.strip()[1:].decode("hex")
                numbers = map(ord,bytes)
                reclen = numbers[0]
                offset = numbers[1]*256 + numbers[2]
                rectype = numbers[3]
                data = numbers[4:-1]
                checksum = reduce(operator.add,numbers) & 0xff
                if ((checksum != 0) or (reclen != len(data))):
                    raise HexFileError("Bad checksum")
            else:
                raise HexFileError("Bad format")
            #so far, so good. Now to slot the data in...
            if (rectype not in (0,1,4)):
                # rectype not known. Fall over
                raise HexFileError("Unknown record format")
            if (rectype == 0):
                self.program[page+offset:page+offset+reclen] = data
            if (rectype == 1):
                #end of file
                return None
            if (rectype == 4):
                if offset != 0:
                    #really should be zero...
                    raise HexFileError("Bad format")
                if reclen != 2:
                    #incorrect length
                    raise HexFileError("Bad format")
                page = data[0]*0x1000000 + data[1]*0x10000
                page  = page | 0x80000000
                    
    def __len__(self):
        return len(self.program.elements)

class ProgrammerError(Exception):
    pass
class Programmer:
    def __init__(self,vID = 0x04d8, pID = 0x000a, configuration = 0, interface = 0):
        self.vID = vID
        self.pID = pID
        self.configuration = configuration
        self.interface = interface
        self.connect()
        
    def connect(self):
        for bus in usb.busses():
            for dev in bus.devices:
                if (dev.idVendor == self.vID) & (dev.idProduct == self.pID):
                    handle = dev.open()
                    handle.setConfiguration(self.configuration)
                    #handle.claimInterface(interface)
                    self.handle = handle
                    chip_info = self.read_data(GET_CHIP_INFO,0,20)
                    chip_info = struct.pack('20B',*chip_info)
                    chip_data = struct.unpack('4I2B2x',chip_info)
                    self.user_range = chip_data[0:2]
                    self.config_range = [x & 0x1f000000 for x in chip_data[2:4]]
                    self.bytes_per_row = chip_data[4]*chip_data[5]
                    return None
        raise ProgrammerError("Bootloader not found")
        
    def read_data(self,command,address,size,timeout=10000, allow_fewer=False):
        index=0
        if address > 0xFFFF:
            index = address >> 16
            address = address & 0xFFFF
        buf = self.handle.controlMsg(usb.ENDPOINT_IN | usb.TYPE_VENDOR | usb.RECIP_OTHER,command,size,value=address,index=index,timeout=timeout)
        if len(buf) != size and not allow_fewer:
            print len(buf)
            raise ProgrammerError("Error reading data : only got %d bytes, expecting %d" % (len(buf),size))
        return buf

    def write_data(self,command,address,data,index=0,timeout=1000):
    #construct data package - size is 64 bits...
        if address > 0xFFFF:
            index = address >> 16
            address = address & 0xFFFF
        r = self.handle.controlMsg(usb.ENDPOINT_OUT | usb.TYPE_VENDOR | usb.RECIP_OTHER,command,data,value=address,index=index,timeout=timeout)
        if r != len(data):
            raise ProgrammerError("Error writing data")
        return r

    def write_program(self,hexfile,set_range=None,set_progress=None):
        #this is to write 64 byte blocks...
        self.write_data(CLEAR_FLASH,0,[],timeout=10000)
        if set_range:
            set_range(len(hexfile.program))
        for i in range(self.user_range[0],self.user_range[1],self.bytes_per_row):
            #check to see if there is any data...
            subset = hexfile.program[i:i+self.bytes_per_row]
            if subset.count(None) < self.bytes_per_row:
                sys.stdout.write(".")
                sys.stdout.flush()
                clean_list(subset)
                self.write_data(SEND_DATA,i,subset)
           	
            if set_progress:
                set_progress(i)
        sys.stdout.write("\n")
		
    def verify_program(self,hexfile,set_range=None,set_progress=None):
        if set_range:
            set_range(len(hexfile.program))
        for i in range(self.user_range[0],self.user_range[1],self.bytes_per_row):
            #check to see if there is any data...
            subset = hexfile.program[i:i+self.bytes_per_row]
            if subset.count(None) < self.bytes_per_row:
                clean_list(subset)
                count = 3
                sys.stdout.write(".")
                sys.stdout.flush()
                while count>0:
                    try:
                        pic_data = self.read_data(REQUEST_DATA,i,self.bytes_per_row)
                        break
                    except IOError as e:
                    	print e
                        print "Glitch in communications. Trying again"
                        self.handle.reset()
                        count -= 1
                for j in range(self.bytes_per_row):
                    if (pic_data[j]!=subset[j]):
                        raise ProgrammerError("Mismatch found at 0x%X (0x%x != 0x%x)" % (i+j,pic_data[j],subset[j]))
            if set_progress:
                set_progress(i)
        sys.stdout.write("\n")

    def reset(self):
        self.write_data(SEND_RESET,0,[0])
        
    def write_i2c(self,address,data):
        return self.write_data(WRITE_I2C_DATA,address,data)
        
    def read_i2c(self,address,size):
        return self.read_data(READ_I2C_DATA,address,size)
        
    def read_i2c_address(self,address,register,size,word=False):
        if word:
            self.write_i2c(address,[register>>8, register & 0xff])
        else:
            self.write_i2c(address,[register])
        return self.read_i2c(address,size)

    def check_i2c(self,address):
        return self.read_data(CHECK_I2C_READY,address,1)
        
    def read_eeprom_data(self,address,length):
        return self.read_data(READ_EEPROM_DATA,address,length)
        
    def write_display(self,page,column,data):
        return self.write_data(WRITE_DISPLAY,address=column,index=page,data=data)
        
    def read_datetime(self):
    
    	tm = struct.unpack("i",struct.pack("4B",*self.read_data(READ_DATETIME,0,4)))
        print tm
        dt = datetime.datetime.fromtimestamp(tm[0])
        print dt
        return dt
        
    def write_datetime(self,dt):
    	tm = time.mktime(dt.utctimetuple())
        return self.write_data(WRITE_DATETIME,0,struct.pack("i",tm))
        
    def read_uart(self):
        
        return ''.join(chr(x) for x in self.read_data(READ_UART, 0, 16, allow_fewer=True))
    
    def write_uart(self, text):
        return self.write_data(WRITE_UART, 0, [ord(x) for x in text])
        

if __name__=="__main__":
    p = Programmer()
    p.write_uart("O")
    time.sleep(0.3)
    print p.read_uart()