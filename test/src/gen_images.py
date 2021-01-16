#!/usr/bin/python
from gen_font import chunk
from itertools import groupby
from glob import glob
import re

files = glob('images/*.xbm')

def read_xbm(fname):
    with open(fname) as f:
        bits = []
        for line in f:
            bits += [int(x,16) for x in re.findall(r'0x[0-9a-fA-F]{2}',line)]
        return bits

with open('images.h','w') as header_file:
    with open('images.c','w') as data_file:
        for f in files:
            var_name = 'image_{}'.format(f.replace('.xbm','').replace('images/',''))
            bits = read_xbm(f)
            header_file.write('extern const char {}[];\n'.format(var_name))
            compressed = []
            for line in chunk(bits,16,0xff):
                #convert to runs of ones and zeroes
                binary = ''.join(''.join(reversed("{:08b}".format(x))) for x in line)
                compressed += [len(list(v)) for k,v in groupby(binary)]
            data_file.write('const char {}[] = {{'.format(var_name)+','.join("0x%02X"%x for x in compressed) + '};\n\n')
            print '{} : {}'.format(var_name,len(compressed))
exit()



#test decompress
i = 0;
character = 0
text = ''
for x in compressed:
    text += str(character)*x
    character = character ^ 1
    i +=x
    if i>=128:
        #newline
        i = 0
        print text
        text = ''
        character = 0
