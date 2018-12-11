#include <stdint.h>
#include "memory.h"
#include "storage.h"

int write_data(void *destination,  const void *source, int length){
    int i, res;
    char *dest = destination;
    const int *src = source;
    for (i=0; i< length; i+=8) {
        res = write_dword(dest, src);
        dest+=8;
        src+=2; 
    }
    return res;
}
