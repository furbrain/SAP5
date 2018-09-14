#!/usr/bin/python

from SimpleXMLRPCServer import SimpleXMLRPCServer
import subprocess
import os
import time
import signal

MDB_INIT = """
device PIC32MM0256GPM028
hwtool sim
set oscillator.frequency 24
set oscillator.frequencyunit Mega
set oscillator.rcfrequency 250
set oscillator.rcfrequencyunit Kilo
set uart2io.uartioenabled true
set uart2io.output file
set uart2io.outputfile /tmp/mdb_fifo
"""

def run(pipe, name):
    try:
        os.unlink('/tmp/mdb_fifo')
    except OSError:
        pass
    open('/tmp/mdb_fifo','w').close()
    pipe.write("halt\n")
    pipe.write("reset\n")
    pipe.write("program %s\n" % name)
    pipe.write("run\n")
    results = ''
    abort_time = time.time()+10
    with open('/tmp/mdb_fifo','r') as f:
        while True:
            results += f.read()
            if '\nOK\n' in results[-15:]:
                return results
            time.sleep(0.1)
            if time.time()>abort_time:
                break
        pipe.write("halt\n")
        return "###TIMED OUT###" + results
        

with open(os.devnull,"w") as devnull:
  with open("mdb_server.log","w") as logfile:
    mdb_process = subprocess.Popen(["mdb"], stdin=subprocess.PIPE, stdout=logfile, stderr=devnull)
    mdb_process.stdin.write(MDB_INIT)
    
    server = SimpleXMLRPCServer(("localhost", 8888), logRequests=False)
    server.finished = False
    def exit_handler():
        server.finished = True
        return 1
    server.register_introspection_functions()
    server.register_function(lambda x: run(mdb_process.stdin, x), 'run')
    server.register_function(exit_handler, 'exit')
    while not server.finished:
        server.handle_request()
    mdb_process.stdin.write("quit\n")
    time.sleep(1)
    mdb_process.kill()
    exit() 

