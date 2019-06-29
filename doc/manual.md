# Shetland Attack Pony v5


The Shetland Attack Pony (SAP) now has an integrated laser range finder, and comes with a OLED display and a wide
variety of beeping noises.

## General Usage


**To turn on:**<br>
Press the button

**To turn off:**<br>
Press the button twice in quick succession

**To navigate in menus:**<br>
Tilt the device briefly towards you - this will move on to the 
next menu item. Clicking the button will select that menu item

__DO NOT SQUEEZE THE DEVICE:__ this can cause errors, especially during calibration. Gently hold it
by the edges. Squeezing the top and bottom surfaces can move internal components and significantly affect accuracy


__DO NOT LOOK DIRECTLY INTO THE LASER BEAM:__ this can cause permanent
eye damage, although a brief exposure of less than a second is
unlikely to cause harm.

## Measurement


The device starts on the measurement screen - a laser symbol.
Top left shows the time (currently unreliable), and top right shows current battery
The bottom row shows the current measurement style and units

To take a reading:
Press and hold the button at any time (this also works in all the menus)

Once the reading is taken, you can scroll through the various bits of info by tipping the device away from you.
By default you are shown the compass bearing of the leg just taken, the inclination, the distance, and the
horizontal distance (extension)

### Store

Pressing the button while on this menu takes you to the leg storage menu. You can select the stations
for the current leg (* means a splay leg).

### Discard


Discard the current reading and return to the measurement screen

### Off

Turn the device off

## Accessing the main menu


To access the main menu from the measurement screen press the button briefly.
To access the main menu from the reading menu press the button on any of the information
(bearing, inclination, distance, extension) items.

### Measure


Return to the measurement screen

### Calibrate


#### Sensors

This is the main calibration routine. Remember to hold the device __gently__ by the edges.

1. Place the device flat against an __inclined__ surface and wait for the beep. 
After each beep rotate through 90 degrees - it will take four readings in total.
The positions do not have to be exact, but the device must be still for the half-second before each beep.

2. Place the device flat on a level(ish) surface. After the first beep turn it so the laser is pointing upwards.
After second beep place it upside down. After the third beep place it pointing downwards
The positions do not have to be exact, but the device must be still for the half-second before each beep.

3. Hold the laser against a solid point with the laser on a fixed point at least 2 metres away. After each beep
rotate by about 45 degrees, while keeping the laser pointing at the same place. There will be eight
readings in total. It is important that you keep the laser pointing at the same point for each reading.

The screen will then display some accuracy statistics - ideally the final number should be 1.00 or less.
If the accuracy is good enough, then the calibration will be saved to the devices memory.

#### Laser

This allows you to calibrate the distance returned by the laser. You should only need to do
this if you add anything to the rear of the device or if you want to measure from the front
of the device.

1. Before selecting this option measure a distance of exactly 1m to a flat surface.
2. After selecting this option, place the device so the point you want to measure from is exactly
1m from the flat surface.
3. The device will make 10 beeps and display the new offset for the laser and the error number - this should
be 0.002 or less.

#### Axes

This allows the device to work out which sensors are pointing in which direction. You should not
need to do this unless you have an experimental device.

### Settings

#### Units (default: metric)

Choose between *metric* (metres) and *imperial* (decimal feet).

#### Style

##### Polar (default)
Directional readings are shown in degrees.

##### Grad
Directional readings are shown in grads. There are 400 grads in a circle.

##### Cartesian
Instead of compass and clino readings, the northing, easting, and vertical distances are
shown. This can aid sketching when used with graph paper. It is also the format
that data is stored internally

#### Display (default day)
Choose between *night* and *day*. Currently not implemented.

#### Timeout (default 30s)
Select the length of time after which the device turns itself off without any input.

#### Set Date
Sets the current date. For each digit in the date, flip the device away or towards
you to set the correct date. Move on to the next digit by pressing the button.
Please note timekeeping is currently very inaccurate

#### Set Time
Sets the current time. For each digit in the time, flip the device away or towards
you to set the correct date. Move on to the next digit by pressing the button.
Please note timekeeping is currently very inaccurate

### Visualise
If you have been storing your recorded legs, this allows you to see what you have recorded so far.
You will see all the legs recorded in the curent survey, move the device to get a different perspective.
Hold the device horizontal and you will see it in plan view. Hold it vertically and you will get an elevation.

Not fully tested

### Debug
This menu item holds several entries to inspect the internal state of the device. Several of 
these refer to the various axes of the device. The y-axis is in the direction of the laser beam. The z-axis goes
up through the top of the device, and the x-axis is at right-angles to both of these.


#### Raw
Show the raw sensor readings. Magnetic readings are in μT, gravitational readings are in g (1g = 9.81ms<sup>-2</sup>).

#### Calibrated
Show the calibrated readings, the top reading is the total field strength (in μT for magnetism, g for gravity).

#### Bearings
Show the current compass and clin readings for the device. Press on the top of the device to see the effect
of squeezing it too tightly!

### Off
Turns the device off

## Battery Life
The main draw on the battery is the laser pointer. From a full charge
the device will remain on continuously for about 2½ hours, assuming 15 seconds to read out the compass and clino, 3s
of laser usage to identify the target station and 3s of reading. You should get approx 300 sets of readings,
but your actual results will vary depending on how long you keep the laser on for and
the ambient temperature.

When the battery is getting low you will first get a warning when the device turns on. When the battery is
critically low, the device will not turn on and just make an "error" noise.

## Known Issues

* The device is sensitive to pressure on the top surface. You can see the effect
of this by choosing Debug->Bearings, and then applying pressure to the top of the device.

* Time is inaccurate. The PCB manufacturer put the timing chips on back to front
and so we have to rely on the inaccurate internal CPU clock.

* Night and day mode are ineffective.

* Visualisation mode is not thoroughly tested.
