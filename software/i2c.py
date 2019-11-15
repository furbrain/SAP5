#!/usr/bin/python3

import time
import SH1106
import bootloader
import sys
import datetime

try:
    print("Connecting to device")
    prog = bootloader.Programmer()
    print("Device found, resetting")
    print(prog.read_i2c(0x3c,1))
except bootloader.ProgrammerError as e:
    print(e)
    sys.exit(1)
sys.exit(0)
