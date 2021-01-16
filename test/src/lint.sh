#!/bin/sh
infer capture --force-integration gcc -- /opt/microchip/xc32/v2.05/bin/xc32-gcc beep.c -I/opt/microchip/xc32/v2.05/pic32mx/include/ -D__32MM0256GPM028__ -D__LANGUAGE_C__ -I/home/phil/Projects/gsl-2.5/

