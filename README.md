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


Debug
------------

### Nemiver

You can debug the firmware by connecting the watch in usb.
You also need to enable DEBUG in "make config".

Then launch this:

	make debug
	nemiver --remote=127.0.0.1:2000 --gdb-binary=/usr/local/bin/msp430-gdb build/openchronos.elf

You can now open the source files and add breakpoints, ...

The first command start gdb of the watch and allow remote connections to it.
The second one start the graphical debugger named "Nemiver" and connect it directly to gdb.

See: http://projects.gnome.org/nemiver/