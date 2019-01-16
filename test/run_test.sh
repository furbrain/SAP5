#!/bin/sh
rm -f /tmp/ceedling_test
mdb << EOF >/dev/null
device PIC32MM0256GPM028
hwtool sim
set oscillator.frequency 24
set oscillator.frequencyunit Mega
set oscillator.rcfrequency 250
set oscillator.rcfrequencyunit Kilo
set uart2io.uartioenabled true
set uart2io.output file
set uart2io.outputfile /tmp/ceedling_test
program $1
run
wait 10000
quit
EOF
cat /tmp/ceedling_test
exit 1

