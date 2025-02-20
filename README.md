# beagle-client

## Purpose

This is a passion project, which is intended to be the software client portion of a system; the goal of which is to modernize outdated or outmoded arcade save game systems leveraging such things as magnetic cards or memory cards.

This project is intended to be run on dedicated hardware, such as a raspberry pi or orangepi, with an NFC reader attached to an arcade machine. It will communicate with both a simulator device and with a remote server, which will store the save data to be retrieved at multiple locations. Ultimately, I would like to construct a service and mobile app to fulfill the usability of this.

It's intended that in a release, it would include include binary distributions or install instructions for emulators/simulators which have an API it can leverage

## TODO
- [ ] Make the application configuration loading for simulators more flexible
- [ ] See if I can clean up the NFC routines, it's kind of gross looking.
- [ ] Write a configurator tool of some sort for the above to configure a distribution
- [ ] Plug into EVMU somehow for games that support VMU save transfer such as Virtual-On OT 5.66, Marvel Vs. Capcom 2, or Power Stone 2.
- [ ] Figure out some kind of PS/PS2 memory card simulator for games which supported that.

## Compiling
Only compiling on linux is supported at the moment.
valid targets are x86_64 an arm64.

You'll need cmake, libarchive, and libnfc.

you may need to change paths in CMakeLists.txt if you're building for arm64.

## Using the software
It's somewhat untested. I'm at the point where I need to actually construct some hardware and do some real testing, however it does seem to work with NFC cards as intended.