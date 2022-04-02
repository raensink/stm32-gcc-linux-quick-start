
# stm32-gcc-linux-quick-start

## MCU VTOR Folder
This folder provides the processor specific vector table and the reset handler.
All are written for the GCC tool chain.

#### Reset Handler
This handler is invoked by the processor hardware in response to a Reset System Exception.
It is always the first exception listed in the vector table.
It's purpose is to:
* Put the processor hardware into a known predictable working state.
* Perform the C Language Startup Initialization.
* Call the application's main function.


#### Vector Table
The vector table is a list of addresses, one for each exception handler function.
The meaning of each position within the table is hardware specific
and corresponds to a specific exception that will be raised by the mcu hardware.
The position and name of each exception handler is standardized by ARM and the silicon vendor,
in this case: ST Micro.

The only exception to this (no pun intended) is the very first entry.
The first entry is used as the initial value of the stack pointer.

When implementing an application specific exception handler,
rather than changing the function name in the vector table itself,
a much better practice is to define, for each handler, a weak aliases to a default handler.
The project specific handler should be defined using the very same name as the one in the table.
At link time, the linker will override any weak definition with
any other function in the application that has the same name as a handler in the vector table.
In which case, the hardware will call the application specific handler
rather than the default handler.

This file also contains the definition of the default handler mentioned above.
It simply loops forever to allow for debug.
In a production system, this approach for the default handler would be OK only when
a watchdog timer is present to alert and recover the system when the default it called.

This vector table is based on the following file from the STM32Cube MCU package:
* Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f091xc.s


