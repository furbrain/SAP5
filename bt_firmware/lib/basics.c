/*
 * basics.c
 *
 *  Created on: 13 Sep 2020
 *      Author: phil
 */

#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


static uint8_t heap[4096];
caddr_t _sbrk(int32_t incr)
{
  static uint8_t *heap_end;
  uint8_t *prev_heap_end;

  if (heap_end == 0)
    heap_end = &heap[0];

  prev_heap_end = heap_end;
  if ((heap_end + incr) > (heap+sizeof(heap)))
  {
//    write(1, "Heap and stack collision\n", 25);
//    abort();
    errno = ENOMEM;
    return (caddr_t) -1;
  }

  heap_end += incr;

  return (caddr_t) prev_heap_end;
}


int _close(int32_t file)
{
  return -1;
}


int _fstat(int32_t file, struct stat *st)
{
	  return -1;
}

int _isatty(int32_t file)
{
  return 1;
}

int _lseek(int32_t file, int32_t ptr, int32_t dir)
{
  return -1;
}

int _write(int fd, char *str, int len)
{
return -1;
}

int _read(int handle, unsigned char * buffer, size_t size)
{
return -1;
}


