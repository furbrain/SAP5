v 20130925 2
C 41400 51800 1 0 0 connector5-1.sym
{
T 43200 53300 5 10 0 0 0 0 1
device=CONNECTOR_5
T 41500 53500 5 10 1 1 0 0 1
refdes=J4
T 41400 51800 5 10 0 1 0 0 1
footprint=CUI_UJ2-MIBH-4-SMT-TR-67
T 41400 51800 5 10 0 1 0 0 1
model=UJ2-MIBH-4-SMT-TR-67
T 41400 51800 5 10 0 1 0 0 1
model-name=ST-USB-001GUB-MC5BR3-SDWP604-4S-TF
}
C 43400 53200 1 0 0 generic-power.sym
{
T 43600 53450 5 10 0 1 0 3 1
net=Vusb:1
T 43400 53500 5 10 1 1 0 0 1
value=Vusb
}
N 43600 53200 43100 53200 4
C 43800 52800 1 0 0 io-1.sym
{
T 44000 52800 5 10 0 1 0 0 1
net=DMINUS:1
T 44000 53400 5 10 0 0 0 0 1
device=none
T 44400 53000 5 10 1 1 180 0 1
value=D-
}
C 43800 52500 1 0 0 io-1.sym
{
T 44000 52500 5 10 0 1 0 0 1
net=DPLUS:1
T 44000 53100 5 10 0 0 0 0 1
device=none
T 44400 52700 5 10 1 1 180 0 1
value=D+
}
C 43500 51700 1 0 0 gnd-1.sym
N 43600 52000 43100 52000 4
N 43100 52600 43800 52600 4
N 43800 52900 43100 52900 4
