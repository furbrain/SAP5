# Shetland Attack Pony v5


The Shetland Attack Pony (SAP) now has an integrated laser range finder, and comes with a OLED display, optional bluetooth capability and a wide variety of beeping noises.

[Demo video on YouTube](http://www.youtube.com/watch?v=q_M8_QxNhjs)


## Get Started


**To turn on:**<br>
Press the button

**To turn off:**<br>
Press the button twice in quick succession

**Take a measurement**<br>
Press and hold the button

**To access the menu from the ready to measure screen**<br>
Shake the device

**Navigating menus:**<br>
Tilt the device briefly towards you - this will move on to the 
next menu item. Clicking the button will select that menu item

__DO NOT LOOK DIRECTLY INTO THE LASER BEAM:__ this can cause permanent
eye damage, although a brief exposure of less than a second is
unlikely to cause harm.

## Introduction

The SAP has three main operating modes: 

* [Ready to Measure](#Ready to Measure)
* [Data](#Data)
* [Menu](#Menu)

In many of these modes you will have top and bottom information rows.
Top left shows the time, and top right shows current battery. If you have a bluetooth module installed, you will see a bluetooth symbol, which will indicate whether the device is connected via bluetooth or not. 
The bottom row shows the current measurement style and units


## Ready to Measure

This is the first mode you will meet on starting the SAP. You will see a laser symbol, indicating the SAP is ready to take a reading. When taking a measurement, make sure the SAP is not near any sources of magnetic interference -  headlights, watches, metal anchors.

Press and hold the button to take a reading, keeping the laser dot lined up on your target. The SAP will make a single beep if the reading was successful, and will enter the [Data mode](#Data).

If the reading was unsuccessful you will hear an error noise and the display will briefly display the cause of the error, then return to Ready to Measure.

To directly access [Menu mode](#Menu), just shake the device.

## Data

This mode shows the data from the most recent measurement. If you have set the [display style](#Display style) to Compact then everything will be visible on a single screen. If not, you will need to scroll between the various readings. The default is to display compass reading in degreees, followed by clinometer reading, then distance in metres and finally extension (horizontal distance). You can choose different units of length and angle in [Settings -> Units](#Units), and you can also choose to just view offset in terms of Easting, Northing and Vertical in [Settings -> Style](#Style).
If you press the button on any of the data screens you will return to [Ready to Measure](#Ready to Measure).

There are a few additional options:

### Store
This allows you to store the data to the memory of the SAP and later retrieve it as a survex file using PonyTrainer. If you select this item, it will default to choosing the next leg (e.g) 1 -> 2 or 2->1. You can also select a splay leg with 1-> - or 2 -> -. If neither of these are suitable you can choose custom and specify which two stations this leg connects.

### Discard
Discards the data and return to [Ready to Measure](#Ready to Measure).

### Main Menu
Go to the [Menu](#Menu).

## Menu
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
that data is stored internally.

#### Display
##### Compact
When you have taken a reading, show all the data on a single screen
##### Large
When you have taken a readingm, show each data item on it's own screen.

#### Timeout (default 30s)
Select the length of time after which the device turns itself off without any input.

#### Set Date
Sets the current date. For each digit in the date, flip the device away or towards
you to set the correct date. Move on to the next digit by pressing the button.

#### Set Time
Sets the current time. For each digit in the time, flip the device away or towards you to set the correct date. Move on to the next digit by pressing the button.

### Calibrate
This option allows you to calibrate the various sensors within the device. With normal operation, you should only need to use the Calibrate Sensors item - you should do this every few months, or whenever you change general location so that the strength and dip of the magnetic field has changed

#### Sensors
At each stage the screen will give instructions.

1. Place the device flat against an __inclined__ surface and wait for the beep. 
After each beep rotate through 90 degrees - it will take four readings in total.
The positions do not have to be exact, but the device must be still for the half-second before each beep.

2. Place the device flat on a level(ish) surface. After the first beep turn it so the laser is pointing straight up.
After second beep place it with the display facing down. After the third beep place it so the laser is pointing downwards
The positions do not have to be exact, but the device must be still for the half-second before each beep.

3. Hold the laser against a solid point with the laser on a fixed point at least 2 metres away. After each beep
rotate by about 45 degrees, while keeping the laser pointing at the same place. There will be eight
readings in total. It is important that you keep the laser pointing at the same point for each reading.

4. Repeat step 3 using different points, ideally about 90 degrees from the previous set.

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

### Measure
Return to [Ready to Measure](#Ready to Measure)

### Visualise
If you have been storing your recorded legs, this allows you to see what you have recorded so far.
You will see all the legs recorded in the curent survey, move the device to get a different perspective.
Hold the device horizontal and you will see it in plan view. Hold it vertically and you will get an elevation.

This feature is still in development.

### Info
This menu item shows several screens with basic information. It can allow some troubleshooting. Several of 
these refer to the various axes of the device. The y-axis is in the direction of the laser beam. The z-axis goes
up through the top of the device, and the x-axis is at right-angles to both of these.

#### Raw
Show the raw sensor readings. Magnetic readings are in μT, gravitational readings are in g (1g = 9.81ms<sup>-2</sup>).

#### Calibrated
Show the calibrated readings, the top reading is the total field strength (in μT for magnetism, g for gravity).

#### Bearings
Show the current compass and clin readings for the device. Also displays the device name

There are also screens that show the exact battery voltage and firmware/hardware versions

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

* Visualisation mode is not thoroughly tested.
