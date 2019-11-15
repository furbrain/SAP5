#!/usr/bin/python3
import time
import freetype
import pprint
import string
from itertools import zip_longest
DEBUG = False

def chunk(data,chunk_size,padding):
    return list(zip_longest(*[iter(data)]*chunk_size,fillvalue = padding))

def transpose_bytes(data):
    if len(data)!=8:
        print("not enough/too many bytes")
        return
    result = [0]*8
    for i in range(8):
        for j in range(8):
            result[7-j] = result[7-j]*2+((data[7-i]>>j)&0x1)
    return result


if DEBUG:
    def sleep(duration):
        if duration==0:
            print("Sleeping for 0 seconds")
        elif duration<0.000001:
            print("Sleeping for %g nanoseconds" % (duration/0.000000001))
        elif duration<0.001:
            print("Sleeping for %g microseconds" % (duration/0.000001))
        elif duration<1:
            print("Sleeping for %g milliseconds" % (duration/0.001))
    time.sleep = sleep

        
us = 0.000001 #microseconds

class Display(object):
    address = 0x3c
    def __init__(self,programmer):
        self.programmer = programmer
        self.top_page = 0
        self.scroll_rate = 0.01
        self.inverted = False
        self.buffer = [[0]*132 for page in range(8)]
        self.cur_column = 0
        self.cur_page = 0
        
    #low level functions    
        
    def send_command(self,command, *data):
        data_set = [0x00,command]
        data_set.extend(data)
        self.programmer.write_i2c(self.address,data_set)
    
    def send_data(self,data):
        self.buffer[self.cur_page][self.cur_column:self.cur_column+len(data)] = data
        self.programmer.write_display(self.cur_page,self.cur_column,data)
            
    def send_pages(self,data,page=0,column=0):
        for cur_page,data_line in enumerate(data):
            real_page = (page+cur_page+self.top_page) % 8
            self.set_page(real_page)
            self.set_column(column)
            self.send_data(data_line)

    #high level functions
    def power_up(self):
        #set control pins high
        self.send_command(0xAD)
        self.send_command(0x8B)
        time.sleep(0.5)
        self.send_command(0xA8,0x3F)
        self.send_command(0XD5,0x50)
        self.send_command(0xDA,0x12)
        self.send_command(0xAF)
        if self.inverted:
            self.send_command(0xA1)
            self.send_command(0xC8)
        else:
            self.send_command(0xA0)
            self.send_command(0xC0)
        self.set_brightness(0xFF)
        self.send_command(0xD3,0)
        self.invert(False)
    
        #ok ready!
        
    def power_down(self):
        self.send_command(0xAE)
        time.sleep(0.1)
        #ok, powered down
        
    def set_row_count(self,row_count):
        """set total number of rows to be displayed"""
        self.send_command(0xA8,row_count-1)
        
    def set_brightness(self,brightness):
        """set brightness level, 0-255"""
        self.send_command(0x81,brightness)

    def set_column(self,column):
        if 0 <= column < 132:
            self.cur_column = column
        else:
            print("invalid column")
        
    def set_page(self,page):
        if 0 <= page < 8: 
            self.cur_page = page
        else:
            print("invalid page!")
            
    def flip(self,flip = None):
        #get all data first
        if flip is None:
            flip = True ^ self.inverted
        if flip:
            self.send_command(0xA1)
            self.send_command(0xC8)
        else:
            self.send_command(0xA0)
            self.send_command(0xC0)
        self.inverted = flip
        #send a copy of our frame buffer...
        self.set_column(0)
        for page in range(8):
            self.set_page(page)
            self.send_data(self.buffer[page])

    def roll(self):
        for i in range(64):
            self.send_command(0xD3,63-i)
            time.sleep(0.02)
            
    def slide(self):
        for i in range(64):
            self.send_command(0x40+i)
            time.sleep(0.1)
        
    def throb(self):    
        for i in range(256):
            self.set_brightness(i)
            time.sleep(0.005)
        for i in range(256):
            self.send_command(0x81,255-i)
            time.sleep(0.005)

    def invert(self,option=True):
        if option:
            self.send_command(0xA7)
        else:
            self.send_command(0xA6)

    def clear_screen(self,byte=0x00):
        for page in range(8):
            self.set_page(page)
            self.send_data([byte]*132)
    
    def scroll_page(self,data=None,up=True,rate=None):
        if rate is None:
            rate = self.scroll_rate
        #first clear all data
        if up:
            self.set_page(self.top_page)
            self.set_column(0)
            self.send_data([0]*132)
            # now rotate it
            for top_line in range(8):
                self.send_command(0xD3,(self.top_page*8+top_line+1) % 64)
                if data:
                    self.set_column(0)
                    mask = ((1<<(top_line+1))-1)
                    self.send_data([x& mask for x in data])
                time.sleep(rate)
            self.top_page = (self.top_page+1) % 8
        else:
            self.set_page((self.top_page-1)%8)
            self.set_column(0)
            self.send_data([0]*132)
            # now rotate it
            for top_line in range(8):
                self.send_command(0xD3,(self.top_page*8-(top_line+1)) % 64)
                if data:
                    self.set_column(0)
                    mask = (256-(1<<(8-top_line)))
                    self.send_data([(x & mask) for x in data])
                time.sleep(rate)
            self.top_page = (self.top_page-1) % 8
        if data:
            self.set_column(0)
            self.send_data(data)
            
    def scroll_pages(self,data,up=True,rate=None):
        if up:
            for page in data:
                self.scroll_page(page,up,rate)
        else:
            for page in reversed(data):
                self.scroll_page(page,up,rate)
                
    def swipe_pages(self,start_page,data,left=True,rate=None,start_col=0,end_col=132,step=8):
        if rate is None:
            rate = self.scroll_rate
        if left:
            for offset in range(start_col,end_col,step):
                for page,data_page in enumerate(data):
                    out = [0]*(end_col-start_col)
                    real_page = (start_page+self.top_page+page) % 8
                    old_slice = self.buffer[real_page][start_col+step:end_col-offset]
                    out[:len(old_slice)] = old_slice
                    new_slice = data[page][:offset]
                    out[end_col-offset:end_col-offset+len(new_slice)] = new_slice
                    self.set_page(real_page)
                    self.set_column(start_col)
                    self.send_data(out)
                time.sleep(rate)
        else:
            for offset in reversed(range(start_col,end_col,step)):
                for page,data_page in enumerate(data):
                    real_page = (start_page+self.top_page+page) % 8
                    out = [0]*(end_col-start_col)
                    old_slice = self.buffer[real_page][end_col-offset:end_col-step]
                    out[end_col-len(old_slice):end_col] = old_slice
                    new_slice = data[page][offset:]
                    out[:len(new_slice)] = new_slice
                    self.set_page(real_page)
                    self.set_column(start_col)
                    self.send_data(out)
                time.sleep(rate)
        
                
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

def get_paged_text(text,spacing=0,offset=0):
    glyphs = [all_glyphs[x] for x in text]
    max_pages = max(x.pages for x in glyphs)
    results = []
    for p in range(max_pages):
        results.append([0]*offset)
        for g in glyphs:
            if p<g.pages:
                results[p].extend(g.bitmaps[p]+[0]*spacing)
            else:
                results[p].extend([0]*(g.width+spacing))
    return results
    
        
def write_text(text,column=0,page=0):
    pages = get_paged_text(text)
    for i,p in enumerate(pages):
             disp.set_page(page+i)
             disp.set_column(column)
             disp.send_data(p)
     
def horiz_line(page,offset):
    disp.set_page(page)
    disp.set_column(0)
    disp.send_data([1<<offset]*128)

def vert_line(column):
    for i in range(8):
        disp.set_page(i)
        disp.set_column(column)
        disp.send_data([0xFF])

#face = freetype.Face('/usr/share/fonts/X11/misc/ter-u32n_unicode.pcf.gz')
#face.set_char_size(face.available_sizes[0].size)
face = freetype.Face('/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf')
face.set_char_size(24*64)
all_glyphs = dict((x,Glyph(x,face)) for x in string.printable)

if __name__=="__main__":
    import bootloader
    p = bootloader.Programmer()
    disp  = Display(p)
    disp.power_up()
    disp.clear_screen(0x00)
    disp.send_command(0xA5)
    time.sleep(1)
    disp.send_command(0xA4)
    horiz_line(0,0)
    horiz_line(7,7)
    vert_line(2)
    vert_line(129)
    time.sleep(5)
    write_text('Testings',2,0)
    write_text('Beesting',2,4)
    time.sleep(2)
    exit()
    words = ('Alpha','Beta','Gamma','Delta','Epsilon')
    for word in words:
        disp.scroll_pages(get_paged_text(word),up=False,rate=0.0001)
    time.sleep(2)
    disp.invert()
    time.sleep(1)
    disp.invert(False)
    ##disp.flip()
    ##disp.clear_screen()
    ##write_text()
    #time.sleep(1)
    #disp.roll()
    #disp.roll()
    ##disp.slide()
    disp.throb()
    disp.power_down()
