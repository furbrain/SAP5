#!/usr/bin/python

import xmlrpclib
import socket
import subprocess
import time
import sys
import os
s = xmlrpclib.ServerProxy('http://localhost:8888')

try:    
    print s.run(sys.argv[1])
except socket.error:
    pid = os.fork()
    if pid > 0:
        subprocess.Popen(['python','mdb_server.py'], 
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
        sys.exit()
    time.sleep(2)
    print s.run(sys.argv[1])


