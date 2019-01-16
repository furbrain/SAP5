v 20130925 2
C 45200 49600 1 0 0 PIC32MM0256GPM028.sym
{
T 45800 56000 5 10 1 1 0 6 1
refdes=U1
T 45600 56000 5 10 0 0 0 0 1
device=PIC32MM0256GPM028
T 45600 56200 5 10 0 0 0 0 1
footprint=SSOP28
T 45200 49600 5 10 0 1 0 0 1
model=PIC32MM0256GPM028-I/SS
T 45200 49600 5 10 0 1 0 0 1
model-name=PIC32MM0256GPM028-I/SS
}
C 41200 56600 1 0 0 generic-power.sym
{
T 41400 56850 5 10 0 1 0 3 1
net=Vcpu:1
T 41200 56900 5 10 1 1 0 0 1
value=Vcpu
}
N 41400 56600 41400 50500 4
N 41400 50500 45300 50500 4
C 49500 52200 1 0 0 generic-power.sym
{
T 49700 52450 5 10 0 1 0 3 1
net=Vcpu:1
T 49500 52500 5 10 1 1 0 0 1
value=Vcpu
}
N 49700 52200 49700 50100 4
N 49700 50100 47700 50100 4
C 48300 50300 1 0 0 capacitor-1.sym
{
T 48500 51000 5 10 0 0 0 0 1
device=CAPACITOR
T 48400 50600 5 10 1 1 0 0 1
refdes=C1
T 48500 51200 5 10 0 0 0 0 1
symversion=0.1
T 48900 50300 5 10 1 1 0 0 1
value=0.1uF
T 48300 50300 5 10 0 1 0 0 1
footprint=0402
T 48300 50300 5 10 0 1 0 0 1
model=302010024
T 48300 50300 5 10 0 1 0 0 1
model-name=CC0402KRX7R8BB104
}
C 47900 49400 1 0 0 gnd-1.sym
N 48000 49700 48000 50500 4
N 47700 50500 48300 50500 4
N 49200 50500 49700 50500 4
C 44700 48800 1 0 0 EMBEDDEDgnd-1.sym
[
P 44800 48900 44800 49100 1 0 1
{
T 44858 48961 5 4 0 1 0 0 1
pinnumber=1
T 44858 48961 5 4 0 0 0 0 1
pinseq=1
T 44858 48961 5 4 0 1 0 0 1
pinlabel=1
T 44858 48961 5 4 0 1 0 0 1
pintype=pwr
}
L 44700 48900 44900 48900 3 0 0 0 -1 -1
L 44755 48850 44845 48850 3 0 0 0 -1 -1
L 44780 48810 44820 48810 3 0 0 0 -1 -1
T 45000 48850 8 10 0 0 0 0 1
net=GND:1
]
N 44800 49100 44800 54400 4
N 44800 52500 45300 52500 4
C 48300 53100 1 0 0 capacitor-1.sym
{
T 48500 53800 5 10 0 0 0 0 1
device=CAPACITOR
T 48400 53400 5 10 1 1 0 0 1
refdes=C3
T 48500 54000 5 10 0 0 0 0 1
symversion=0.1
T 48900 53100 5 10 1 1 0 0 1
value=10uF
T 48300 53100 5 10 1 1 0 0 1
footprint=0805
T 48300 53100 5 10 1 1 0 0 1
model=302010178
T 48300 53100 5 10 1 1 0 0 1
model-name=CC0805KKX5R8BB106
}
N 48300 53300 47700 53300 4
C 49700 53000 1 0 0 gnd-1.sym
N 49800 53300 49200 53300 4
C 48100 54000 1 0 0 io-1.sym
{
T 48300 54000 5 10 0 1 0 0 1
net=SDA:1
T 48300 54600 5 10 0 0 0 0 1
device=none
T 49000 54100 5 10 0 1 0 1 1
value=IO
T 48800 54200 5 10 1 1 180 0 1
value=SDA
}
N 48100 54100 47700 54100 4
C 48100 54400 1 0 0 output-2.sym
{
T 49000 54600 5 10 0 0 0 0 1
net=SCL:1
T 48300 55100 5 10 0 0 0 0 1
device=none
T 48400 54500 5 10 1 1 0 1 1
value=SCL
}
N 48100 54500 47700 54500 4
C 48000 52800 1 0 0 io-1.sym
{
T 48200 52800 5 10 0 1 0 0 1
net=DMINUS:1
T 48200 53400 5 10 0 0 0 0 1
device=none
T 48600 53000 5 10 1 1 180 0 1
value=D-
}
N 48000 52900 47700 52900 4
C 48000 52400 1 0 0 io-1.sym
{
T 48200 52400 5 10 0 1 0 0 1
net=DPLUS:1
T 48200 53000 5 10 0 0 0 0 1
device=none
T 48600 52600 5 10 1 1 180 0 1
value=D+
}
N 48000 52500 47700 52500 4
C 43100 55200 1 0 0 resistor-1.sym
{
T 43400 55600 5 10 0 0 0 0 1
device=RESISTOR
T 43300 55500 5 10 1 1 0 0 1
refdes=R1
T 43600 55500 5 10 1 1 0 0 1
value=10k
T 43100 55200 5 10 0 1 0 0 1
footprint=0402
T 43100 55200 5 10 0 1 0 0 1
model=301010004
T 43100 55200 5 10 0 1 0 0 1
model-name=RC0402JR-0710KL
}
C 44300 55200 1 0 0 resistor-1.sym
{
T 44600 55600 5 10 0 0 0 0 1
device=RESISTOR
T 44500 55500 5 10 1 1 0 0 1
refdes=R2
T 44900 55500 5 10 1 1 0 0 1
value=1k
T 44300 55200 5 10 0 1 0 0 1
footprint=0402
T 44300 55200 5 10 0 1 0 0 1
model=301011989
T 44300 55200 5 10 0 1 0 0 1
model-name=RC0402FR-071KL
}
N 41400 55300 43100 55300 4
N 44000 55300 44300 55300 4
N 45300 55300 45200 55300 4
C 44400 54400 1 90 0 capacitor-1.sym
{
T 43700 54600 5 10 0 0 90 0 1
device=CAPACITOR
T 44100 55100 5 10 1 1 180 0 1
refdes=C4
T 43500 54600 5 10 0 0 90 0 1
symversion=0.1
T 44100 54700 5 10 1 1 180 0 1
value=0.1uF
T 44400 54400 5 10 0 1 0 0 1
footprint=0402
T 44400 54400 5 10 0 1 0 0 1
model=302010024
T 44400 54400 5 10 0 1 0 0 1
model-name=CC0402KRX7R8BB104
}
N 44800 54400 44200 54400 4
C 48000 55300 1 0 0 generic-power.sym
{
T 48200 55550 5 10 0 1 0 3 1
net=Vusb:1
T 48000 55600 5 10 1 1 0 0 1
value=Vusb
}
N 48200 55300 47700 55300 4
N 47700 52100 49700 52100 4
C 49400 51000 1 180 0 input-2.sym
{
T 49400 50800 5 10 0 0 180 0 1
net=Rx:1
T 48800 50300 5 10 0 0 180 0 1
device=none
T 48900 50900 5 10 1 1 180 7 1
value=Rx
}
N 48000 50900 47700 50900 4
C 48000 51200 1 0 0 output-2.sym
{
T 48900 51400 5 10 0 0 0 0 1
net=Tx:1
T 48200 51900 5 10 0 0 0 0 1
device=none
T 48900 51300 5 10 1 1 0 1 1
value=Tx
}
N 48000 51300 47700 51300 4
C 36900 54100 1 0 0 connector6-1.sym
{
T 38700 55900 5 10 0 0 0 0 1
device=CONNECTOR_6
T 37000 56100 5 10 1 1 0 0 1
refdes=J1
T 36900 54100 5 10 0 1 0 0 1
footprint=JUMPER6
}
N 38600 55800 45200 55800 4
N 45200 55800 45200 55300 4
N 38600 55500 41400 55500 4
N 38600 55200 41000 55200 4
N 41000 55200 41000 49300 4
N 41000 49300 44800 49300 4
N 38600 54900 40600 54900 4
N 40600 54100 40600 54900 4
N 40600 54100 45300 54100 4
N 38600 54600 40300 54600 4
N 40300 53700 40300 54600 4
N 40300 53700 45300 53700 4
C 43900 53200 1 0 0 resistor-1.sym
{
T 44200 53600 5 10 0 0 0 0 1
device=RESISTOR
T 44100 53500 5 10 1 1 0 0 1
refdes=R3
T 44400 53500 5 10 1 1 0 0 1
value=1M
T 43900 53200 5 10 0 1 0 0 1
footprint=0402
T 43900 53200 5 10 0 1 0 0 1
model=301011990
T 43900 53200 5 10 0 1 0 0 1
model-name=RC0402FR-071ML
}
N 43900 53300 43900 52900 4
N 43900 52900 45300 52900 4
C 38800 53300 1 0 0 generic-power.sym
{
T 39000 53550 5 10 0 1 0 3 1
net=Vbat:1
T 38800 53600 5 10 1 1 0 0 1
value=Vbat
}
C 48900 53700 1 0 0 switch-spst-1.sym
{
T 49300 54400 5 10 0 0 0 0 1
device=SPST
T 49200 54000 5 10 1 1 0 0 1
refdes=J2
T 48900 53700 5 10 0 1 0 0 1
footprint=JUMPER2
}
N 47700 53700 48900 53700 4
N 49700 53700 49800 53700 4
N 49800 53700 49800 53300 4
C 48100 54800 1 0 0 output-2.sym
{
T 49000 55000 5 10 0 0 0 0 1
net=PERIPH_EN:1
T 48300 55500 5 10 0 0 0 0 1
device=none
T 49000 54900 5 10 1 1 0 1 1
value=PERIPH_EN
}
N 48100 54900 47700 54900 4
C 43000 53200 1 0 0 resistor-1.sym
{
T 43300 53600 5 10 0 0 0 0 1
device=RESISTOR
T 43200 53500 5 10 1 1 0 0 1
refdes=R4
T 43500 53500 5 10 1 1 0 0 1
value=1M
T 43000 53200 5 10 0 1 0 0 1
footprint=0402
T 43000 53200 5 10 0 1 0 0 1
model=301011990
T 43000 53200 5 10 0 1 0 0 1
model-name=RC0402FR-071ML
}
C 39800 53200 1 0 0 resistor-1.sym
{
T 40100 53600 5 10 0 0 0 0 1
device=RESISTOR
T 40000 53500 5 10 1 1 0 0 1
refdes=R5
T 40300 53500 5 10 1 1 0 0 1
value=1M
T 39800 53200 5 10 0 1 0 0 1
footprint=0402
T 39800 53200 5 10 0 1 0 0 1
model=301011990
T 39800 53200 5 10 0 1 0 0 1
model-name=RC0402FR-071ML
}
N 39000 53300 39800 53300 4
N 40700 53300 43000 53300 4
C 44600 51500 1 90 0 capacitor-1.sym
{
T 43900 51700 5 10 0 0 90 0 1
device=CAPACITOR
T 44300 52200 5 10 1 1 180 0 1
refdes=C21
T 43700 51700 5 10 0 0 90 0 1
symversion=0.1
T 44300 51800 5 10 1 1 180 0 1
value=0.1uF
T 44600 51500 5 10 0 1 0 0 1
footprint=0402
T 44600 51500 5 10 0 1 0 0 1
model=302010024
T 44600 51500 5 10 0 1 0 0 1
model-name=CC0402KRX7R8BB104
}
N 44400 52400 44400 52900 4
C 42800 50000 1 0 0 input-2.sym
{
T 42800 50200 5 10 0 0 0 0 1
net=CHG:1
T 43400 50700 5 10 0 0 0 0 1
device=none
T 43300 50100 5 10 1 1 0 7 1
value=CHG
}
N 45200 50100 44200 50100 4
C 38900 54200 1 0 0 resistor-1.sym
{
T 39200 54600 5 10 0 0 0 0 1
device=RESISTOR
T 39000 54000 5 10 1 1 0 0 1
refdes=R7
T 39500 54000 5 10 1 1 0 0 1
value=1k
T 38900 54200 5 10 0 1 0 0 1
footprint=0402
T 38900 54200 5 10 0 1 0 0 1
model=301011990
T 38900 54200 5 10 0 1 0 0 1
model-name=RC0402FR-071ML
}
N 38900 54300 38600 54300 4
N 39800 54300 40300 54300 4
C 41600 50800 1 0 0 SiT1630.sym
{
T 43400 52100 5 10 1 1 0 6 1
refdes=U10
T 42000 52300 5 10 0 0 0 0 1
device=SiT1630
T 42000 52500 5 10 0 0 0 0 1
footprint=SiT1630
T 41600 50800 5 10 1 1 0 0 1
model=SiT1630AE-H6-DCC-32.768G 
T 41600 50800 5 10 1 1 0 0 1
model-name=SiT1630AE-H6-DCC-32.768G 
}
N 41700 51600 41400 51600 4
N 42700 50900 42700 49300 4
N 43700 51600 43700 50900 4
N 43700 50900 45300 50900 4
N 44400 51500 44400 49300 4
C 42100 49400 1 90 0 capacitor-1.sym
{
T 41400 49600 5 10 0 0 90 0 1
device=CAPACITOR
T 41800 50200 5 10 1 1 180 0 1
refdes=C2
T 41200 49600 5 10 0 0 90 0 1
symversion=0.1
T 41800 49700 5 10 1 1 180 0 1
value=0.1uF
T 42100 49400 5 10 0 1 90 0 1
footprint=0402
T 42100 49400 5 10 0 0 90 0 1
model-name=CC0402KRX7R8BB104
T 42100 49400 5 10 0 0 90 0 1
model=302010024
}
N 41900 50300 41900 50500 4
N 41900 49400 41900 49300 4
