# stm32-gcc-linux-quick-start
Hello World, LED Blinky, and more!

This is an STM32 quick-start project with the following characteristics:
- Development Platform: Linux/WSL;
- Build Tool-Chain: gcc-arm-none-eabi & make;
- Refactored software architecture using great engineering practices (or at least better than what you find in most examples out there);
- FreeRTOS;
- Support for mutiple applications (executables) using a common shared platform;
- Support for multiple MCUs;
- Support for multiple boards;

This is currently a work-in-progress, and maybe so for quite a while longer.
The plan is to create create a minimal, yet well organized, and ridiculously well commented
project that blinks an LED and more.  Aspirational platform features include:
- Software trace log to external serial UART.
- On-Time support
- CLI monitor
- Other basic features needed by many embedded projects.

## Current Status
#### MCU Support:
- STM32F091RC
- STM32L476RG
- more to come!
#### Board Support:
- NUCLEO-F091RC
- NUCLEO-L476RG
- more to come!

## Getting started
The following instructions assume you are using Linux: WSL (my fav), virtual machine, or native.
#### Clone
This repo has submodules, so be sure to clone with the option shown below.
```
git clone  git@github.com:raensink/stm32-gcc-linux-quick-start.git  --recurse-submodule
```
#### Example Build
To build the button-blinky-it application:
```
cd stm32-gcc-linux-quick-start
make --makefile=apps/button-blinky-it/Makefile clean
make --makefile=apps/button-blinky-it/Makefile build
make --makefile=apps/button-blinky-it/Makefile rebuild
```
#### Example Load
Because WSL does not yet support USB ports, we use a separate linux host to physically connect
with the embedded target. We call this host the Device Test Agent (DTA).
This arrangement also allows for the embedded target board to be located at a distance:
in another room or even in another building; all you need is a network connection to the DTA.

To load the button-blinky-it application:
```
export DTA_HOST=dta-hostname-from-my-ssh-config-file
./tools/load-remote-target --appl button-blinky-it
```
#### Run
To run the button-blinky-it application:
```
Press blue user button to change the flash rate;
Connect external LEDs to Morpho connector CN7 pins 35-38;
```

## Top Level Folders
#### apps
One or more applications supported by this project.
Multiple application builds may exist in this project folder concurrently,
but only one at a time may be loaded and executed on the target.

#### platform
Future.  Reusable components that support the applications.

#### core
Core embedded features and capabilities necessary for many embedded projects.

#### mcu
Startup code and low-level drivers specific to a particular microcontroller.

#### tools
Develoment tools for building, deploying, and testing this project.

#### build
This is a temporary folder that is automatically created by the makefiles.
All derived build products exist in this directory.
This folder is excluded from the repo using the .gitignore file.
A 'make clean' operation will delete this entire folder.


