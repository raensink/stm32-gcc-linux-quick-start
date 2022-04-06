
/*
================================================================================================*=
Reset Handler

This is a project-specific version of the CMSIS Reset_Handler function.
This handler is invoked by the processor hardware in response to a Reset System Exception.
The purpose of the reset handler is to:
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

According to the CMSIS documentation, the CMSIS startup file may require application specific
adaptations and therefore should be copied into the application project folder.
That is exactly what we have done here, at least in part.

This reset handler is based on the startup file from the STM32Cube MCU Packages:
    STM32CubeF0/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f091xc.s


DEPENDENCIES:
    This code assumes the following symbols have been defined in the linker script:

    _estack  Initial value of the SP, typically at the end of RAM.

    _sidata  Start address of the non-zero-initialized data in Flash.
    _sdata   Start address of the non-zero-initialized data in RAM.
    _edata   End  address  of the non-zero-initialized data in RAM.

    _sbss    Start address of the BSS zero-initialized data in RAM.
    _ebss    End address of the BSS zero-initialized data in RAM.

    This handler will 'relocate' RW Data from Flash to RAM.
    Initialized RW Data is initialized using values that have been programmed into Flash.

    Zero-Initialized RW Data is initialized programmatically.
    BSS = "Block Started by Symbol" or "Better Save Space".
    There is no need to store all these zeros in Flash; we only need to know
    the size of this section so we can allocate the space for it
    and then initialize all of it to zero at runtime.

System Initialization:
    Often, the reset handler is used to call a system initialization function
    that configures the clock tree and other system core control peripherals.
    We choose to focus the reset handler on the bare minimum necessary to setup
    the C Run-Time enviroment and leave everything else to the application
    to control and configure.

C Run-Time
    A call to __libc_init_array is necessary to
    put the system into the correct state for running C programs.

Main:
    Upon completion, the reset handler will call the application "main".

SPDX-License-Identifier: MIT-0
================================================================================================*=
*/
        .syntax  unified     @ Use the unified (ARM & THUMB) assembler syntax (ARM Dependent)
        .cpu     cortex-m4   @ Specify the target processor. (ARM Dependent)
        .fpu     softvfp     @ Specify the floating point format. (ARM Dependent)
        .thumb               @ Generate the Thumb instruction set. (ARM Dependent)


/*
--------------------------------------------------------------------------------+-
BACKGROUND

RW Data is 'relocated' to RAM during startup.
Initialized RW Data is initialized using values that have been loaded into Flash.
The following values are defined by the linker script.

_sidata  Start address of the initialized data in Flash.
_sdata   Start address of the initialized data in RAM.
_edata   End address of the initialized data in RAM.

Zero-Initialized RW Data is initialized programmatically by the startup code.
BSS = "Block Started by Symbol" or "Better Save Space".
There is no need to store all these zeros in Flash; we only need to know
the size of this section so we can allocate and initialize it at runtime.

_sbss    Start address of the BSS data in RAM.
_ebss    End address of the BSS data in RAM.
--------------------------------------------------------------------------------+-
StartDataFlash: .word  _sidata
StartDataRAM:   .word  _sdata
EndDataRAM:     .word  _edata

StartBSSRAM:    .word  _sbss
EndBSSRAM:      .word  _ebss
--------------------------------------------------------------------------------+-
*/


//
// as an alternative see:
// __STATIC_FORCEINLINE __NO_RETURN void __cmsis_start(void)
// in cmsis_gcc.h
//

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
Reset Handler Function

We are writing this in assembler because the C run-time has not yet been setup.
CMSIS has deprecated this approach, yet ST is still doing it this way.

This function is analogous to the crt0.s "_start" function in libc.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
        .section   .text.Reset_Handler        @ in its own section, after other text.
        .type      Reset_Handler, %function   @ not sure why the linker cares about this.
        .global    Reset_Handler              @ give this symbol external linkage.
        .weak      Reset_Handler              @ allow the linker to redefine this with a strong version.

Reset_Handler:

        /* ---------------------- Set Stack Pointer Register ----------------------------- */
        ldr     sp, =_estack      @ Use the symbol defined in the linker script.

        /* ---------------------- CMSIS Startup Init -------------------------------------- */
        @ Call and return from our equivalents for the CMSIS SystemInit function.
        @@ bl      MCUCORE_SCB_MaybeEnableFPU
        @@ bl      MCUCORE_CLOCK_StartupInit
        @ TODO: maybe move the Vector Table.
        @ MCUCORE_VTOR_SetVectorTableLocation(void)

        /* ---------------------- Non-Zero Initialized Data ------------------------------ */
        movs    r1, 0             @ R1 is the index from the start of the initialized data.
        b       LoopCopyDataInit  @ First check whether there even is any initialized data.

CopyDataInit:                     @ Copy one word of initialized data from Flash to RAM.
        ldr     r3, =_sidata      @ R3 is the address in Flash of the first word of non-zero initialized data.
        ldr     r3, [r3, r1]      @ Index R3, Load R3 with next word of non-zero initialized data from Flash.
        str     r3, [r0, r1]      @ Store R3 at the next location in RAM.
        adds    r1, r1, 4         @ Increment the index.

LoopCopyDataInit:
        ldr     r0, =_sdata       @ R0 is the address in RAM of the first word of non-zero initialized data.
        ldr     r3, =_edata       @ R3 is the address in RAM just past the end of the non-zero initialized data.
        adds    r2, r0, r1        @ R2 is the address in RAM of the next word in RAM to be written to.
        cmp     r2, r3            @ (Compare is R2-R3)  Have we reached the end of initialized data?
        bcc     CopyDataInit      @ (Branch if carry clear)  If not, branch to copy the next word.

                                  @ We are done with non-zero initialization.
        ldr     r2, =_sbss        @ R2 is the address in RAM of the first word of zero initialized data.
        b       LoopFillZerobss   @ Begin zero initialization.

        /* ---------------------- Zero Initialized Data ---------------------------------- */
FillZerobss:
        movs    r3, 0             @ The zero to be stored.
        str     r3, [r2], 4       @ Store the zero at *R2, Increment R2 by 4

LoopFillZerobss:
        ldr     r3, = _ebss       @ R3 is the address in RAM just past the end of the zero initialized data.
        cmp     r2, r3            @ (Compare is R2-R3)  Have we reached the end of zero initialized data?
        bcc     FillZerobss       @ (Branch if carry clear)  If not, branch to copy the next word.


        /* ------------------------- C RunTime Initialize Data --------------------------- */
                                     @ We are done with non-zero initialization.
        bl      __libc_init_array    @ Call (and return from) C startup initialization.

        /*
        --------------------------------------------------------------------------------+-
        Call Application Main()

        At this point, the "system" must be in a standards-conforming state.
        This means that all the pre-conditions for a C program have been satisfied
        from the perspective of the C Language Standard.
        --------------------------------------------------------------------------------+-
        */
        bl   main    @ Call the application entry point. Appears to be no setup for argc and argv.

        /*
        --------------------------------------------------------------------------------+-
        Loop Forever
        Just in case main returns.

        This approach is fine if we have configured a watchdog timer.
        Otherwise it would be better to take some action here
        to alert and recover the system.
        --------------------------------------------------------------------------------+-
        */
LoopForever:
        b    LoopForever

.size   Reset_Handler, . - Reset_Handler


