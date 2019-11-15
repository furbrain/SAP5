#!/usr/bin/python
# -*- coding: latin-1 -*-
import freetype
import string
from itertools import izip_longest
DEBUG = False

def chunk(data,chunk_size,padding):
    return list(izip_longest(*[iter(data)]*chunk_size,fillvalue = padding))

def transpose_bytes(data):
    if len(data)!=8:
        print "not enough/too many bytes"
        return
    result = [0]*8
    for i in range(8):
        for j in range(8):
            result[7-j] = result[7-j]*2+((data[7-i]>>j)&0x1)
    return result


                
class Glyph(object):
    def __init__(self,char,face):
        self.bitmaps = []
        face.load_char(char,freetype.FT_LOAD_DEFAULT | freetype.FT_LOAD_RENDER | freetype.FT_LOAD_MONOCHROME)
        bitmap = face.glyph.bitmap
        self.width = bitmap.width
        #separate bitmap into rows
        rows = list((chunk(bitmap.buffer,bitmap.pitch,0)))
        #add padding at the top
        rows = [[0]*bitmap.pitch]*((face.size.ascender/64)-face.glyph.bitmap_top) + rows
        #separate rows into pages
        pages = chunk(rows,8,[0]*bitmap.pitch)
        self.pages = len(pages)
        for p in pages:
             blocks = zip(*p)
             cols = []
             for block in blocks:
                 cols.extend(transpose_bytes(block))
             self.bitmaps.append(cols[:bitmap.width])

def get_font_data(face,name,advance):
    all_glyphs = dict((x,Glyph(x,face)) for x in string.printable)
    all_glyphs['`'] = Glyph(chr(0xB0),face)
    all_glyphs['>'] = Glyph(chr(0xBB),face)
    bitmap_index = 0
    bitmaps = ""
    indices = ""
    for x in range(32,128):
        if chr(x) in all_glyphs:
            glyph = all_glyphs[chr(x)]
        else:
            glyph = all_glyphs['~']
        indices += "{0x%02x,0x%0x,0x%04x}, /* %s */\n" % (glyph.width,glyph.pages,bitmap_index,chr(x))
        bitmaps += "/* character: %s */\n" % chr(x)
        if glyph.width:
            for page in glyph.bitmaps:
                bitmaps += ",".join("0x%02x" % x for x in page)+",\n"
        bitmap_index += glyph.width*glyph.pages
    max_pages = max(x.pages for x in all_glyphs.values())
    max_width = max(x.width for x in all_glyphs.values())
    output = string.Template("""
const struct GLYPH_DATA ${name}_indices[96] = {
$indices
};

const char ${name}_bitmaps[] = {
$bitmaps
};

const struct FONT ${name}_font = {$advance,$max_pages,$max_width,${name}_indices,${name}_bitmaps};
""")
    return output.substitute(locals())
    
    
if __name__=="__main__":
    big_face = freetype.Face('/usr/share/fonts/truetype/droid/DroidSans.ttf')
    big_face.set_char_size(26*64)
    little_face = freetype.Face('/usr/share/fonts/X11/misc/ter-u16n_unicode.pcf.gz')
    little_face.set_char_size(little_face.available_sizes[0].size)
    #print len(indices),indices
    #print bitmaps
    output = open('font.c','w')
    output.write('#include "font.h"\n\n')
    output.write(get_font_data(big_face,'large',2))
    output.write(get_font_data(little_face,'small',0))
    output.close()
