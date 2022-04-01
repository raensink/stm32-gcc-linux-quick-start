
# mcu/stm32f0/vtor
This folder provides the baseline vector table and the reset handler.

All are for GCC tool chain.

## vtor Folder Description

#### vtor/vector-table.s
The vector table is a list of addresses, one for each function to handle the corresponding exception.
The only exception to this is the first entry which is used as the initial value of the stack pointer.
The position and name of each exception handler is standardized by ARM and the silicon vendor.

When implementing an exception handler, rather than changing the function name in the table,
the best practice is to define, for each handler, a weak aliases to a default handler.
The project specific handler should be defined using the very same name as the one in the table.
At link time, the linker will override any weak definition with any other function
in the application that has the same name as a handler in the vector table.

This file also contains the definition of the default handler mentioned above.
It simply loops forever to allow for debug.

#### vtor/reset-handler.s
This handler is invoked by the processor hardware in response to a Reset System Exception.
It's purpose is to:
* Put the processor hardware into a known predictable working state.
* Perform the C Language Startup Initialization.
* Call the application's main function.

Details:
* Set Stack Pointer (is this necessary? I thought the hardware did this?)
* Copy initialized data to RAM.
* Zero-Fill the BSS Section.
* Call libc init.
* Call the application main().
* Loop forever just in case.

