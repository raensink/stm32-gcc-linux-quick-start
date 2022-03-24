# stm32-gcc-linux-quick-start
Hello World, Blinky, and more!

This is an STM32 quick-start project with the following characteristics:
- Development Platform: Linux/WSL
- Build Tool-Chain: gcc-arm-none-eabi & make
- Refactored software architecture using best engineering practices (or at least better than what you find in many example projects).
- Support for mutiple applications (executables) using a common shared platform.
- Support for multiple MCUs.
- Support for multiple boards.

As of Mar 2021, this is still a work-in-progress.
The plan is to create create a minimal, yet well organized and ridiculously well commented
project that blinks an LED and more.  Aspirational platform features include:
- Software trace log to external serial UART.
- On-Time support
- CLI monitor
- Other basic features needed by almost every project.

## Current Status
#### MCU Support:
- STM32F091RC
#### Board Support:
- NUCLEO-F091RC

## Getting started
#### Clone
```
git clone  git@github.com:raensink/stm32-gcc-linux-quick-start.git  --recurse-submodule
```
#### Build
```
cd stm32-gcc-linux-quick-start
make --makefile=apps/blinky-f0/Makefile clean
make --makefile=apps/blinky-f0/Makefile all
```
#### Load
```
export DTA_HOST=hostname-from-my-ssh-config-file
./tools/load-remote-target --appl blinky-f0
```
#### Run
```
Press blue user button to change the flash rate;
Connect external LEDs to Morpho connector CN7 pins 35-38;
```

## Top Level Folders
#### apps
One or more applications supported by this project.
Multiple application builds may exist in this project folder concurrently,
but only one at a time may be loaded and executed on the target.

#### build
This is a temporary folder that is automatically created by the build scripts.
All derived build products exist in this directory.
This folder is excluded from the repo using the .gitignore file.

#### board
Future.  Source code that is specific to a particular board.

#### platform
Future.  Reusable components that support the applications.

#### mcu
Source code that is specific to a particular microcontroller.

#### tools
Develoment tools for building, deploying, and testing this project.


