#!/usr/bin/python

from SimpleXMLRPCServer import SimpleXMLRPCServer
import subprocess
import os
import fcntl
import time
import signal
import re

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

class StreamBuffer():
    def __init__(self, in_stream, out_stream):
        fd = in_stream.fileno()
        flag = fcntl.fcntl(fd, fcntl.F_GETFL)
        fcntl.fcntl(fd, fcntl.F_SETFL, flag | os.O_NONBLOCK)
        self.in_stream = in_stream
        self.out_stream = out_stream
        self.buffer = ""
        
    def read(self):
        try:
            data = self.in_stream.read()
        except IOError:
            return  ""
        self.out_stream.write(data)
        self.out_stream.flush()
        self.buffer += data
        return data
        
    def readline(self):
        self.read()
        i = self.buffer.find("\n")
        if i >= 0:
            line = self.buffer[0:i+1]
            self.buffer = self.buffer[i+1:]
            return line
        else:
            return ""
            
        

def run(pipe, buf, name):
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
    error = ''
    abort_time = time.time()+30
    with open('/tmp/mdb_fifo','r') as f:
        while True:
            time.sleep(0.1)
            results += f.read()
            mdb_output = buf.readline()
            if re.match(r"^>?[A-Z]\d{4}", mdb_output):
                error = "###SIMULATOR ERROR###\n" + mdb_output
                break
            if time.time()>abort_time:
                error = "###TIMED OUT###"
                pipe.write("halt\n")
                break
            if '\nOK\n' in results[-15:]:
                return results
            if '\nFAIL\n' in results[-15:]:
                return results
        return results + "\n" + error
        
with open("/dev/null","w") as devnull:
  with open("mdb_server.log","w") as logfile:
    mdb_process = subprocess.Popen(["mdb"], stdin=subprocess.PIPE, 
                                            stdout=subprocess.PIPE, 
                                            stderr=devnull)
    mdb_process.stdin.write(MDB_INIT)
    server = SimpleXMLRPCServer(("localhost", 8888), logRequests=False)
    server.finished = False
    def exit_handler():
        server.finished = True
        return 1
    server.register_introspection_functions()
    buf= StreamBuffer(mdb_process.stdout, logfile)
    server.register_function(lambda x: run(mdb_process.stdin, buf, x), 'run')
    server.register_function(exit_handler, 'exit')
    while not server.finished:
        server.handle_request()
    mdb_process.stdin.write("quit\n")
    time.sleep(1)
    mdb_process.kill()
    exit() 

