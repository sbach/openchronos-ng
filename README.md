openchronos-ngr
===============

Ti-Ez430 Firmware


What's not working
------------

* Accelerometer sensor & module
* Rf PPT, SYNC, ACC modes
* Alarm module


Usage
------------

First you need to configure the firmware:

	make config

Then:

	make

The compiled files are in build/ and the current configuration is in config/.

Now you need to install the generated build/openchronos.txt on the watch. Do:

	make install (method=usb)

By default it install the firmware on the watch using the rf dongle.
With "method=usb" it will use the usb dongle and prog the watch with "mspdebug".


Commands
------------

To clean the generated files of the build:

	make clean

To clean everything generated (build & config files):

	make clear

To create the Doxygen documentation from source code:

	make doc

Also:

	make run
	make debug