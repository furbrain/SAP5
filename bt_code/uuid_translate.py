#!/usr/bin/env python3

import sys


def grouper(iterable, n, fillvalue=None):
    "Collect data into fixed-length chunks or blocks"
    # grouper('ABCDEFG', 3, 'x') --> ABC DEF Gxx
    args = [iter(iterable)] * n
    return zip(*args)

text = sys.argv[1]
text =text.replace("-","")
chunked = list(grouper(text,2))
print(", ".join(f"0x{a}{b}" for a,b in reversed(chunked)))
