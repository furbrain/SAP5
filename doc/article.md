Non-linear calibration of a digital compass and accelerometer

#Introduction

The Shetland Attack Pony (SAP) is a cave surveying tool, which has undergone several iterations in its lifetime **FOOTNOTE**. The original version was the first electronic compass/clinometer developed for caving  In its current form it consists of a 32bit CPU (PIC32MM0256GPM028 **FOOTNOTE**), a lithium-ion battery, an OLED display, a laser range finder, an accelerometer (LSM6DS3 by ST **FOOTNOTE**) and a magnetometer (BM1422AGMV by Rohm **FOOTNOTE**). This is an open source project - all the code and hardware designs can be found on Github.

#Principle of Operation

##Laser rangefinder
The laser rangefinder uses a simple time-of-flight algorithm - it sends out pulses of laser light and records the time taken for those pulses to be received back. These are typically accurate to +/- 1mm,

##Magnetometer
The BM1422AGMV uses 3 orthogonal magneto impedance sensors. These consist of a thin film of soft ferromagnetic materials with the special property that their impedance to high frequency alternating current is highly sensitive to any external magnetic fields. The associated IC therefore generates a high frequency alternating current and measures the impedance to determine the magnetic field. This specific sensor is accurate to +/-0.042 uT (Earth's horizontal magnetic field component is typically up to 30 uT, depending on location)


##Accelerometer
The LSM6DS3 use a micro-electro-mechanical system (MEMS) to measure acceleration. This consists of a small mass of silicon on a cantilever between two plates. Under acceleration, the mass moves towards one of the plates, altering the capacitance between the mass and the plate. The LSM6DS3 consists of 3 of these assemblies in an orthogonal pattern. It is accurate to 90 ug (where one g is 9.81 ms^-2)

##Priniciple of determining orientation

###Coordinate system
We shall imagine the SAP initially to be lying flat on a plane with the display upwards. The y coordinate extends in the positive direction parallel to the laser. The z-coordinate extends in a positive direction up through the display. The x-coordinate extends in a positive direction to the left when looking down at the display with the laser pointing away from you 

###Determining orientation
We can use the Magnetometer to determine a vector for gravity g and magnetism m, we can therefore calculate vectors for East and North using the cross-product. 

E = G^ X M^
N = E^ X G^ 

O = (E^ N^ G^)

The matrix O then forms an orthogonal basis and we can use this to translate the y-vector (i.e. the direction of the laser beam) (0,1,0) into real world coordinates y'

y' = yO^T
azimuth = atan2(Y'_1, Y'_0)
inclination = atan2(|(y'_0,y'_1)|, y'_2)

#Traditional calibration

The above calculations of course assume that we have perfect sensors, each exactly aligned, and that there is no magnetic interference from any other components. Unfortunately, none of these assumptions are true. We can divide the potential sources of error into errors intrinsic to the sensors and extrinsic errors

##Intrinsic Errors
Each sensor has an offset β_sensor and a scale factor A_sensor, and also a degree of non-linearity. Non-linearity is the degree to which the sensor reading departs from the straight line, we shall denote it here as psi(x). 

x_sensor = A_sensor x_raw + β_sensor + psi(x_raw)

##Extrinsic Errors
The incident magnetic field on the sensor can be affected by both hard iron β_sensor (which is a simple offset) and soft iron effects A_soft iron(which can be a shear and scale error) within the device.Each sensor can also be rotated with respect both to the ideal orthogonal position, so we also have a 3x3 rotation matrix R for each set of sensors

x = R . A_soft_iron (x_sensor + β_hard_iron)

For the time being, we will ignore the non-linearity term, and we can simplify all this to 

x = A(x_raw + β) 

where A is a 3x3 matrix and β is a 3-vector

##Basic calibration process
We can make a first estimation at A and β by noting that both the gravitational and magnetic fields in any single location should be constant (at least over the time taken to calibrate the device). We take a series of 24 readings, of which eight are distributed in multiple directions and orientations. The other 16 are two sets of eight readings all taken from the same origin to two destination points, with the device held in varying degrees of roll along the y axis. This gives us a cloud of points which are distributed on the surface of an ellipsoid. We can use the process described 


#Issues with non-linearity
