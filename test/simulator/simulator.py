import numpy as np
from PIL import Image
import pyaudio
import struct
import time
import quaternion as q

COLOR = (200,200,255)
class Input:
    NONE         = 0
    FLIP_DOWN    = 1
    FLIP_UP      = 2
    FLIP_LEFT    = 3
    FLIP_RIGHT   = 4
    SINGLE_CLICK = 5
    LONG_CLICK   = 6
    DOUBLE_CLICK = 7


class I2CDevice:
    def __init__(self):
        self.current = 0
        self.registers = bytearray([0]*256)

    def read(self, length):
        return self.registers[self.current:self.current+length]
        
    def write(self, address, data):
        self.current = address
        self.registers[self.current:self.current+len(data)] = data
        self.current += len(data)
        return 0

class Display (I2CDevice):
    COMMAND = {
        0xA0 : ("horizontal_invert", False),
        0xA1 : ("horizontal_invert", True),
        0xA4 : ("all_on", False),
        0xA5 : ("all_on", True),
        0xA6 : ("colour_invert", False),
        0xA7 : ("colour_invert", True),
        0xAE : ("on", False),
        0xAF : ("on", True),
        0XC0 : ("vertical_invert", False),
        0XC8 : ("vertical_invert", True),
    }
    COMMAND.update({x : ("column_lo", x) for x in range(16)})
    COMMAND.update({x+0x10 : ("column_hi", x) for x in range(16)})
    COMMAND.update({x+0x40 : ("start_line", x) for x in range(64)})
    COMMAND.update({x+0xB0 : ("page", x) for x in range(16)})
    
    def __init__(self):
        self.column_lo = 0
        self.column_hi = 0
        self.page = 0
        self.vertical_invert = False
        self.horizontal_invert = False
        self.colour_invert = False
        self.all_on = False
        self.on = False
        self.data = np.zeros((128, 8), dtype=np.uint8)
        self.start_line = 0
        
        I2CDevice.__init__(self)
        
    def write_image(self, data):
        column = self.column_hi*16+self.column_lo
        for x in data:
            if column < 128:
                self.data[column, self.page] = x
            column = (column+1) % 132
        self.column_lo = column % 16
        self.column_hi = column >> 4
        
    def do_command(self, data):
        setattr(self, self.COMMAND[data][0], self.COMMAND[data][1])
        
    def write(self, address, data):
        if address==0:
            if data[0] in self.COMMAND:
                self.do_command(data[0])
            return 0
        elif address==0x40:
            self.write_image(data)
            return 0
        else:
            return 255
            
    def get_image(self):
        if not self.on:
            data = np.zeros((128,8), dtype=np.uint8)
        elif self.all_on:
            data = np.full((128,8), 255, dtype=np.uint8)
        else:
            data = self.data
        i = Image.frombytes("1", (64,128), data, "raw", "1;R")
        i = i.transpose(Image.ROTATE_90)
        i = i.transpose(Image.FLIP_TOP_BOTTOM)
        #rotate image if needed
        if self.start_line != 0:
            bottom = i.crop((0, 0, 128, self.start_line))
            top = i.crop((0, self.start_line, 128, 64))
            bottom.load()
            top.load()
            delta = 64 - self.start_line
            i.paste(top, (0, 0))
            i.paste(bottom, (0, delta))
        if self.horizontal_invert:
            i = i.transpose(Image.FLIP_LEFT_RIGHT)
        if self.vertical_invert:
            i = i.transpose(Image.FLIP_TOP_BOTTOM)
        return i        
        
            
    
class Sensor(I2CDevice):
    def __init__(self):
        self.grav = [0, 0, -1.0]
        self.mag = [20, 0, -30]
        self.azimuth = 0.0
        self.inclination = 0.0
        self.roll = 0.0
        self.quat = q.one 
        I2CDevice.__init__(self)
        self.set_registers()
        
    def set_registers(self):
        gravity = q.rotate_vectors(self.quat, [0,1,0])
        mag = q.rotate_vectors(self.quat, [0,0.6,-0.8])
        gravity *= 0x4000
        mag *= 800
        vals = np.hstack([gravity, mag]).astype("int")
        struct.pack_into(">3h8x3h",self.registers, 0x3B, 
            -vals[1],
            vals[0],
            -vals[2],
            vals[3],
            -vals[4],
            -vals[5])

    def set_orientation(self, azimuth, inclination, roll):
        self.roll = roll
        self.quat = (q.y * np.deg2rad(roll)/2).exp()
        self.inclination = inclination
        self.quat = self.quat * (q.x * np.deg2rad(inclination)/2).exp()
        self.azimuth = azimuth
        self.quat = self.quat * (q.z * np.deg2rad(azimuth)/2).exp()
        self.set_registers()
        
class Laser:
    def __init__(self):
        self.on = False
        self.distance = 1.0
        self.reply = ""
        
    def write(self, text):
        cmd = text[0]
        if cmd=="O":
            self.on = True
            self.reply = ": OK\n"
        elif cmd=="C":
            self.on = False
            self.reply = ": OK\n"
        elif cmd in "MDF":
            self.on = False
            self.reply = ": %2.3fm\n" % self.distance
        return [3]

        
    def read(self):
        result = bytearray(self.reply)
        self.reply = ""
        return bytearray([4]) + result
        
    def get_len(self):
        return [5, 32-len(self.reply)]
        
class Buzzer:
    def __init__(self, volume=0.5):
        self.volume = volume
        self.fs = 44100
        self.audio = pyaudio.PyAudio()
        self.stream = self.audio.open(format=pyaudio.paFloat32,
                channels=1,
                rate=self.fs,
                output=True,
                frames_per_buffer=256)
        #self.stream.stop_stream()
        
    def beep(self, frequency, duration):
        sample_count = self.fs*duration
        #print frequency, duration, sample_count
        samples = (np.sin(2*np.pi*np.arange(sample_count)*frequency/self.fs)*0.5).astype(np.float32).tobytes()
        self.stream.start_stream()
        self.stream.write(samples)
        self.stream.stop_stream()

    
class Sim:
    def __init__(self):
        self.sensor = Sensor()
        self.laser = Laser()
        self.display = Display()
        self.input = Input.NONE
        self.buzzer = Buzzer()
        
    def set_input(self, value):
        self.input = value
    
        
    def I2CWrite(self, address, command, data):
        if address==0xD0:
            result = self.sensor.write(command, data)
        elif address==0x78:
            result = self.display.write(command, data)
        else:
            print "Weird I2C address %d" % address
            result = 255
        return [1, result]

    def I2CRead(self, address, length):
        if address==0xD1:
            result = self.sensor.read(length)
        elif address==0x79:
            result = self.display.read(length)
        else:
            print "Weird I2C address %d" % address
            result = [0]
        return bytearray([2]) + result

    def handleMessage(self, message):
        code = message[0]
        #Hello
        if code in []:
            print("In : ", message)
        if code==0:
            return [0]
        elif code==1:
            return self.I2CWrite(message[1], message[2], message[3:])
        elif code==2:
            return self.I2CRead(message[1], message[2])
        elif code==3:
            return self.laser.write(bytearray(message[1:]).decode())
        elif code==4:
            return self.laser.read()
        elif code==5:
            return self.laser.get_len()
        elif code==6:
            return [6]
        elif code==7:
            _, freq, duration = struct.unpack_from("cdi", bytearray(message))
            self.buzzer.beep(freq, duration/1000.0)
            return [7,0]
        elif code==8:
            temp = self.input
            self.input = Input.NONE
            return [8, temp]
        elif code==9:
            result = struct.pack("I", time.time())
            return [9] + [ord(x) for x in result]
        else:
            print "Unrecognised protocol %d" % code
            return [255]
    
    def get_image(self):
        return self.display.get_image()
        
if __name__ == "__main__":
    from PIL import Image
    
    s = Sim()
    s.buzzer.beep(200,0.2)
    s.buzzer.beep(400,0.2)
    s.buzzer.beep(600,0.2)
    s.buzzer.beep(800,0.2)
    s.display.write_image([x for x in range(128)])
    s.display.page = 2
    s.display.write_image([192]*128)
    s.display.page = 4
    s.display.write_image([x ^ 255 for x in range(128)])
    s.display.on = True
    s.display.start_line = 4
    i = s.display.get_image()
    i.show()
    
