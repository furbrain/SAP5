import six
import struct

class StructParser:
    ALIGNMENT = 1
    
    def __init__(self, data, fmt=None):
        self.offset = 0
        if fmt is None:
            fmt = self.FMT
        else:
            self.FMT = fmt
        for key, value in fmt:
            if isinstance(value, six.string_types):
                if self.is_list(value):
                    setattr(self, key, list(struct.unpack_from(value, data, self.offset)))
                else:
                    setattr(self, key, struct.unpack_from(value, data, self.offset)[0])
                self.offset += struct.calcsize(value)
            else:
                obj = StructParser(data[self.offset:], value)
                setattr(self, key, obj)
                self.offset += obj.offset
        if self.offset % self.ALIGNMENT:
            self.offset += self.ALIGNMENT - (self.offset % self.ALIGNMENT)

    def is_list(self, value):
        if value[-1]=="s": return False
        if value[0] in '=<>!@':
            char = value[1]
        else:
            char = value[0]
        if char.isdigit(): return True
        
    def is_valid(self):
        raise NotImplemented
        
    def as_str(self, indent=0):
        text = ""
        for key, value in self.FMT:
            if isinstance(value, six.string_types):
                text += " "*indent*4 + key +": " + str(getattr(self, key)) + "\n"
            else:
                text += getattr(self, key).as_str(indent+1)
        return text
        
    def __repr__(self):
        return self.as_str()
                
    @classmethod
    def read_array(cls, data):
        offset = 0
        arr = []
        try:
            while True:
                obj = cls(data[offset:])
                offset += obj.offset
                if obj.is_valid(): arr.append(obj)
        except struct.error:
            pass
        return arr
        
