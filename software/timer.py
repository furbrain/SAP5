#!/usr/bin/python

import sys
import time
import struct

import bootloader

try:
    print "Connecting to device"
    prog = bootloader.Programmer()
    print "Device found, reading data"
    data = prog.read_program(0xBF800040,4)
    x, = struct.unpack_from("I",data)
    print("0x{:08x}\n".format(x))
    print data
    time.sleep(600)
    data = prog.read_program(0xBF800040,4)
    x, = struct.unpack_from("I",data)
    print("0x{:08x}\n".format(x))
    print data
except bootloader.ProgrammerError as e:
    print e
    sys.exit(1)
sys.exit(0)
