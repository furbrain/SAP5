#!/bin/bash

stty -F /dev/serial0 sane
stm32loader -p /dev/serial0 -P none  -w -v -a 0x10040000 bt_code.binary

