#!/usr/bin/python3

import time
import bootloader
import sys
import datetime
    
def show_progress(i):
    if not hasattr(show_progress, "counter"):
         show_progress.counter = 0  # it doesn't exist yet, so initialize it
    i = i/1024
    if i > show_progress.counter:
        disp.set_page(6)
        disp.set_column(show_progress.counter)
        page = [0xff]*(i-show_progress.counter)
        disp.send_data(page)
        show_progress.counter = i
                
try:
    print("Connecting to device")
    prog = bootloader.Programmer()
    print("%s found" % prog.get_name())
    print("Loading hexfile")
    hexfile  = bootloader.HexFile(sys.argv[1])
    print("Programming %d bytes" % len(hexfile))
    prog.write_program(hexfile,set_progress=None)
    print("Verifying %d bytes" % len(hexfile))
    prog.verify_program(hexfile,set_progress=None)
    print("Setting Time")
    prog.write_datetime(datetime.datetime.now())
    print("Programming complete")
except bootloader.ProgrammerError as e:
    print(e)
    sys.exit(1)
sys.exit(0)
