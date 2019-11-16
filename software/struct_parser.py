import six
import struct
import array

class StructParser:
    ALIGNMENT = 1
    
    @classmethod
    def create_empty(cls):
        zero_array = array.array('b',[0]*cls.get_len())
        return cls.from_buffer(zero_array)
    
    @classmethod
    def from_buffer(cls, buff, fmt=None):
        obj = cls()
        offset = 0
        if fmt is None:
            fmt = obj.FMT
        else:
            obj.FMT = fmt
        for key, value in fmt:
            if isinstance(value, six.string_types):
                if obj.is_list(value):
                    setattr(obj, key, list(struct.unpack_from(value, buff, offset)))
                else:
                    setattr(obj, key, struct.unpack_from(value, buff, offset)[0])
                offset += struct.calcsize(value)
            else:
                subobj = StructParser.from_buffer(buff[offset:], value)
                setattr(obj, key, subobj)
                offset += subobj.get_len(value)
        return obj

    @classmethod        
    def get_len(cls, fmt=None):
        if fmt is None:
            fmt = cls.FMT
        length = 0
        for key, value in fmt:
            if isinstance(value, six.string_types):
                length += struct.calcsize(value)
            else:
                length += StructParser.get_len(value)
        if length % cls.ALIGNMENT:
            length += cls.ALIGNMENT - (length % cls.ALIGNMENT)
        return length
        
                    
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
                text += " "*indent*4 + key +":\n"
                text += getattr(self, key).as_str(indent+1)
        return text
        
    def as_dict(self):
        result = {}
        for key, value in self.FMT:
            if isinstance(value, six.string_types):
                result[key] = getattr(self, key)
            else:
                result[key] = getattr(self, key).as_dict()
        return result
    
    @classmethod            
    def from_dict(cls, dct, fmt=None):
        obj = cls()
        if fmt is None:
            fmt = cls.FMT
        else:
            obj.FMT = fmt
        keys = [x[0] for x in fmt]
        if sorted(dct.keys()) != sorted(keys):
            raise ValueError("Dict does not contain correct keys")
        for key, value in fmt:
            if isinstance(value, six.string_types):
                setattr(obj, key, dct[key])
            else:
                subobj = StructParser.from_dict(dct[key], value)
                setattr(obj, key, subobj)
        return obj
                
        
    def __repr__(self):
        return self.as_str()
                
    @classmethod
    def read_array(cls, data):
        offset = 0
        obj_size = cls.get_len()
        arr = []
        try:
            while True:
                obj = cls.from_buffer(data[offset:])
                offset += obj_size
                if obj.is_valid(): arr.append(obj)
        except struct.error:
            pass
        return arr
        
