#!/usr/bin/python

import time
import SH1106
import bootloader
import sys
import datetime

try:
    print "Connecting to device"
    prog = bootloader.Programmer()
    print "Device found, resetting"
    prog.reset()
except bootloader.ProgrammerError as e:
    print e
    sys.exit(1)
sys.exit(0)
