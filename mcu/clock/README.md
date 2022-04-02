
# mcu/STM32F0/core
This directory provides a set of project-specific services for the STM32F0 family of microcontrollers.
At this time, the content has only be tested with the STM32F091RC MCU.
Additional refactorings and/or parameterization of this content may be required after testing with additional hardware.

The scope of this content is closely aligned with some of the startup and system configuration requirements of the CMSIS standard.
Specifically:
* System Clock & SysTick
* Additional requirements as needed in the future.


## Software Modules
#### Clock
The purpose of this module is to provide startup initialization services for the system clock
and to satisfy the clock-related CMSIS dependencies that some drivers expect.


