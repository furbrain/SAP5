The STMicroelectronics BlueNRG-1 is a low power Bluetooth® smart system 
on chip, compliant with the Bluetooth® v4.1 specification and supporting 
master, slave and simultaneous master-and-slave roles.


HARDWARE
This project has been tested on the STMicroelectronics BlueNRG-1 development 
platform (order code: STEVAL-IDB007V1).

The STEVAL-IDB007V1 board provides a set of hardware resources for a wide 
range of application scenarios: sensor data (accelerometer, pressure and 
temperature sensor), remote control (buttons and LEDs) and debug message
management through USB virtual COM port. 


DEBUG
The ST-LINK_gdbserver can from v3.0.3 program and debug BlueNRG-1 devices.


SOFTWARE
The BlueNRG-1 Development Kit Software, available from STMicroelectronics,
provides a Bluetooth low energy binary library with a complete set of APIs 
and related events callbacks allowing to interface with the Bluetooth low 
energy features offered by the BlueNRG-1 device. It also includes a complete 
set of peripheral drivers (header and source files) allowing to interface to 
the device peripherals and a complete set of related demonstrations 
applications (header and source files). These examples contains prepared 
examples for Atollic TrueSTUDIO. 
The BlueNRG-1 Development Kit Software can be downloaded from:
http://www.st.com/content/st_com/en/products/embedded-software/evaluation-tool-software/stsw-bluenrg1-dk.html 


INFO
The Atollic TrueSTUDIO Terminal view can be used to receive and send 
messages from/to the board when the Virtual COM port is used. Please see
the "Hello World" project available in the BlueNRG-1 Development Kit Software.
E.g.
BlueNRG-1 DK 2.1.0\Project\BlueNRG1_Periph_Examples\Micro\Hello_World\TrueSTUDIO 
