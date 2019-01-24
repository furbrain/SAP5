import six
import struct

class StructParser:
    def __init__(self, data, fmt=None):
        self._offset = 0
        if fmt is None:
            fmt = self.__FMT__
        for key, value in fmt:
            if isinstance(value, six.string_types):
                if self.is_list(value):
                    setattr(self, key, list(struct.unpack_from(value, data, self._offset)))
                else:
                    setattr(self, key, struct.unpack_from(value, data, self._offset)[0])
                self._offset += struct.calcsize(value)
            else:
                obj = StructParser(data[self._offset:], value)
                setattr(self, key, obj)
                self._offset += obj._offset
    
                
    def is_list(self, value):
        if value[-1]=="s": return False
        if value[0] in '=<>!@':
            char = value[1]
        else:
            char = value[0]
        if char.isdigit(): return True

