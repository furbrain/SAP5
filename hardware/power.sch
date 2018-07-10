v 20130925 2
C 57600 42800 1 0 0 ground.sym
N 51600 45700 59900 45700 4
N 54800 47300 57400 47300 4
N 56000 46400 56000 47300 4
C 52300 48800 1 0 0 generic-power.sym
{
T 52500 49050 5 10 0 1 0 3 1
net=Vusb:1
T 52300 49100 5 10 1 1 0 0 1
value=Vusb
}
C 56200 47300 1 0 0 generic-power.sym
{
T 56400 47550 5 10 0 1 0 3 1
net=Vbat:1
}
N 58400 45700 58400 46600 4
N 59400 47300 60400 47300 4
C 56200 47300 1 0 0 generic-power.sym
{
T 56400 47550 5 10 0 1 0 3 1
net=Vbat:1
T 56200 47600 5 10 1 1 0 0 1
value=Vbat
}
C 60200 47300 1 0 0 generic-power.sym
{
T 60400 47550 5 10 0 1 0 3 1
net=Vcpu:1
T 60200 47600 5 10 1 1 0 0 1
value=Vcpu
}
C 57300 46000 1 90 0 capacitor-1.sym
{
T 56600 46200 5 10 0 0 90 0 1
device=CAPACITOR
T 56800 46600 5 10 1 1 0 0 1
refdes=C11
T 56400 46200 5 10 0 0 90 0 1
symversion=0.1
T 57200 46100 5 10 1 1 0 0 1
value=4.7uF
T 57300 46000 5 10 0 1 0 0 1
footprint=0805
T 57300 46000 5 10 0 1 0 0 1
model=302010146
T 57300 46000 5 10 0 1 0 0 1
model-name=CC0805KKX5R8BB475
}
C 60100 46000 1 90 0 capacitor-1.sym
{
T 59400 46200 5 10 0 0 90 0 1
device=CAPACITOR
T 59800 46800 5 10 1 1 180 0 1
refdes=C12
T 59200 46200 5 10 0 0 90 0 1
symversion=0.1
T 60000 46100 5 10 1 1 0 0 1
value=4.7uF
T 60100 46000 5 10 0 1 0 0 1
footprint=0805
T 60100 46000 5 10 0 1 0 0 1
model=302010146
T 60100 46000 5 10 0 1 0 0 1
model-name=CC0805KKX5R8BB475
}
N 57100 46900 57100 47300 4
N 57100 46000 57100 45700 4
N 59900 45700 59900 46000 4
N 59900 46900 59900 47300 4
C 57200 43300 1 0 0 XC6228D302VR-G.sym
{
T 59400 45000 5 10 1 1 0 6 1
refdes=U6
T 58200 44400 5 10 1 1 0 0 1
device=LDO
T 57600 45400 5 10 0 0 0 0 1
footprint=SOT25
T 58200 44200 5 10 1 1 0 0 1
value=3.0V
T 57200 43300 5 10 0 1 0 0 1
model=XC6228D302VR-G
T 57200 43300 5 10 0 1 0 0 1
model-name=XC6228D302VR-G
}
N 57300 44500 56700 44500 4
N 56700 44500 56700 47300 4
N 55200 45700 55200 43100 4
N 55200 43100 60100 43100 4
N 58500 43100 58500 43400 4
C 55900 44000 1 0 0 input-2.sym
{
T 55900 44200 5 10 0 0 0 0 1
net=PERIPH_EN:1
T 56500 44700 5 10 0 0 0 0 1
device=none
T 56400 44100 5 10 1 1 0 7 1
value=PERIPH_EN
}
C 56400 44900 1 180 0 capacitor-1.sym
{
T 56200 44200 5 10 0 0 180 0 1
device=CAPACITOR
T 55800 45000 5 10 1 1 180 0 1
refdes=C14
T 56200 44000 5 10 0 0 180 0 1
symversion=0.1
T 56600 45000 5 10 1 1 180 0 1
value=4.7uF
T 56400 44900 5 10 0 1 90 0 1
footprint=0805
T 56400 44900 5 10 0 1 90 0 1
model=302010146
T 56400 44900 5 10 0 1 90 0 1
model-name=CC0805KKX5R8BB475
}
C 60300 43200 1 90 0 capacitor-1.sym
{
T 59600 43400 5 10 0 0 90 0 1
device=CAPACITOR
T 60200 43800 5 10 1 1 0 0 1
refdes=C15
T 59400 43400 5 10 0 0 90 0 1
symversion=0.1
T 60200 43300 5 10 1 1 0 0 1
value=4.7uF
T 60300 43200 5 10 0 1 0 0 1
footprint=0805
T 60300 43200 5 10 0 1 0 0 1
model=302010146
T 60300 43200 5 10 0 1 0 0 1
model-name=CC0805KKX5R8BB475
}
N 55500 44700 55200 44700 4
N 56700 44700 56400 44700 4
N 59700 44500 60100 44500 4
N 60100 44500 60100 44100 4
N 60100 43200 60100 43100 4
N 52500 46700 52500 48800 4
N 52500 45800 52500 45700 4
C 55500 45800 1 0 0 JST-2P-SMD_HW2-SMD-2.0-90D.sym
{
T 56000 46120 5 7 0 0 0 0 1
footprint=HW2-SMD-2.0-90D
T 55400 46200 5 10 1 1 0 0 1
refdes=B1
T 55100 45800 5 10 1 1 0 0 1
device=BATTERY
T 55500 45800 5 10 0 1 0 0 1
model=320110026
T 55500 45800 5 10 0 1 0 0 1
model-name=ST12-0001
}
N 55800 46400 56000 46400 4
N 56000 46200 56000 45700 4
N 55800 46200 56000 46200 4
C 59900 44500 1 0 0 generic-power.sym
{
T 60100 44750 5 10 0 1 0 3 1
net=Vperiph:1
T 59800 44800 5 10 1 1 0 0 1
value=Vperiph
}
C 57300 46500 1 0 0 MC78LC.sym
{
T 59100 47800 5 10 1 1 0 6 1
refdes=U8
T 57700 48000 5 10 0 0 0 0 1
device=MC78LC
}
C 52700 46500 1 0 0 MAX1555.sym
{
T 54500 48200 5 10 1 1 0 6 1
refdes=U9
T 53100 48400 5 10 0 0 0 0 1
device=MAX1555
}
C 52700 45800 1 90 0 capacitor-1.sym
{
T 52000 46000 5 10 0 0 90 0 1
device=CAPACITOR
T 52200 46400 5 10 1 1 0 0 1
refdes=C19
T 51800 46000 5 10 0 0 90 0 1
symversion=0.1
T 52600 45900 5 10 1 1 0 0 1
value=4.7uF
T 52700 45800 5 10 0 1 0 0 1
footprint=0805
T 52700 45800 5 10 0 1 0 0 1
model=302010146
T 52700 45800 5 10 0 1 0 0 1
model-name=CC0805KKX5R8BB475
}
N 52800 47700 52500 47700 4
N 53800 46600 53800 45700 4
C 55100 46000 1 90 0 capacitor-1.sym
{
T 54400 46200 5 10 0 0 90 0 1
device=CAPACITOR
T 54600 46600 5 10 1 1 0 0 1
refdes=C20
T 54200 46200 5 10 0 0 90 0 1
symversion=0.1
T 54400 46100 5 10 1 1 0 0 1
value=4.7uF
T 55100 46000 5 10 0 1 0 0 1
footprint=0805
T 55100 46000 5 10 0 1 0 0 1
model=302010146
T 55100 46000 5 10 0 1 0 0 1
model-name=CC0805KKX5R8BB475
}
N 54900 46900 54900 47300 4
N 54900 46000 54900 45700 4
C 51400 48300 1 270 0 led-1.sym
{
T 52000 47500 5 10 0 0 270 0 1
device=LED
T 51100 48100 5 10 1 1 0 0 1
refdes=LED?
T 52200 47500 5 10 0 0 270 0 1
symversion=0.1
}
C 53100 48600 1 0 0 led-1.sym
{
T 53900 49200 5 10 0 0 0 0 1
device=LED
T 53900 49000 5 10 1 1 0 0 1
refdes=LED?
T 53900 49400 5 10 0 0 0 0 1
symversion=0.1
}
C 51700 45800 1 90 0 resistor-1.sym
{
T 51300 46100 5 10 0 0 90 0 1
device=RESISTOR
T 51400 46600 5 10 1 1 180 0 1
refdes=R?
T 51400 46300 5 10 1 1 180 0 1
value=1k
T 51700 45800 5 10 0 1 90 0 1
footprint=0603
T 51700 45800 5 10 0 1 90 0 1
model=301011989
T 51700 45800 5 10 0 1 90 0 1
model-name=RC0402FR-071KL
}
C 55500 47800 1 90 0 resistor-1.sym
{
T 55100 48100 5 10 0 0 90 0 1
device=RESISTOR
T 55200 48500 5 10 1 1 180 0 1
refdes=R?
T 55800 48500 5 10 1 1 180 0 1
value=1k
T 55500 47800 5 10 0 1 90 0 1
footprint=0603
T 55500 47800 5 10 0 1 90 0 1
model=301011989
T 55500 47800 5 10 0 1 90 0 1
model-name=RC0402FR-071KL
}
N 51600 45700 51600 45800 4
N 51600 46700 51600 47400 4
N 51600 48300 51600 48800 4
N 51600 48800 53100 48800 4
N 54000 48800 55400 48800 4
N 55400 48800 55400 48700 4
N 55400 47800 55400 47700 4
N 55400 47700 54800 47700 4
