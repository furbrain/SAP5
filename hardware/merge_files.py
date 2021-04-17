#!/usr/bin/python3

import sys
import re


def partition(f):
    f = [x.strip() for x in f]
    parts = {}
    parts['header'] = [x for x in f if re.match("M48|METRIC|INCH",x)]
    parts['def']    = [x for x in f if re.match("T\d+C",x)]
    parts['drill']  = [x for x in f if re.match("T\d+$|X\d+Y\d+$",x)]
    return parts
    
a = open(sys.argv[1],'r').readlines()
b = open(sys.argv[2],'r').readlines()


a = partition(a)
b = partition(b)
if a['header'] != b['header']:
    raise ValueError("Headers do not match")
result = a['header'] + a['def'] + b['def'] + ["%"] + a['drill'] + b['drill'] + ["M30"]
print =('\r\n'.join(result))

