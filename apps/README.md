# blinky-f0/apps

This folder contains all of the applications supported by this project.
Each application is built independently from the others using its own makefile that
identifies the source files to be included as well as other build parameters.
The application specific makefiles share a common makefile located here in the apps folder.

Multiple application builds may exist in this project folder concurrently,
but only one at a time may be loaded and executed on the target.


## Common Makefile
Common elements from the application specific makefiles are factored out
and located in the makefile-apps-common.mk file.
This makefile is included by the application specific makefiles.


## Project Applications
#### blinky-f0
Hello world application to blink some LEDs and respond to button presses.


