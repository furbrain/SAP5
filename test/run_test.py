#!/usr/bin/python

import xmlrpclib
import socket
import subprocess
import time
import sys
s = xmlrpclib.ServerProxy('http://localhost:8888')

try:    
    print s.run(sys.argv[1])
except socket.error:
    subprocess.Popen(['python','mdb_server.py'])
    time.sleep(2)
    print s.run(sys.argv[1])


