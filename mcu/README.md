# stm32-gcc-linux-quick-start

## Top Level Folders
#### apps
One or more applications supported by this project.
Multiple application builds may exist in this project folder concurrently,
but only one at a time may be loaded and executed on the target.

#### platform
Future.  Reusable components that support the applications.

#### board
Future.  Source code that is specific to a particular board.

#### core
Core embedded features and capabilities necessary for many embedded projects.

#### mcu
Startup code and low-level drivers specific to a particular microcontroller.

#### tools
Develoment tools for building, deploying, and testing this project.

#### build
This is a temporary folder that is automatically created by the build scripts.
All derived build products exist in this directory.
This folder is excluded from the repo using the .gitignore file.
A make clean operation will delete this entire folder.


