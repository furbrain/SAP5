#!/usr/bin/python

import xmlrpclib
import socket
import subprocess
import time
import sys
s = xmlrpclib.ServerProxy('http://localhost:8888')
s.exit()


