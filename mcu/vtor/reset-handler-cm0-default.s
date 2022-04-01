
/*
================================================================================================*=
Reset Handler

This is a project-specific version of the CMSIS Reset_Handler function.
According to the CMSIS documentation, the CMSIS startup file may require application specific
adaptations and therefore should be copied into the application project folder.
We have done this, in part, by refactoring the Reset_Handler and putting it into its own source file here.

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
================================================================================================*=
*/
        .syntax  unified     @ Use the unified (ARM & THUMB) assembler syntax (ARM Dependent)
        .cpu     cortex-m0   @ Specify the target processor. (ARM Dependent)
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
        @ Set Stack Pointer
        ldr   r0, =_estack
        mov   sp, r0

        @ Copy the data segment initializers from flash to SRAM
        ldr r0, =_sdata
        ldr r1, =_edata
        ldr r2, =_sidata
        movs r3, #0
        b LoopCopyDataInit

CopyDataInit:
        ldr r4, [r2, r3]
        str r4, [r0, r3]
        adds r3, r3, #4

LoopCopyDataInit:
        adds r4, r0, r3
        cmp r4, r1
        bcc CopyDataInit

        @ Zero fill the bss segment.
        ldr r2, =_sbss
        ldr r4, =_ebss
        movs r3, #0
        b LoopFillZerobss

FillZerobss:
        str  r3, [r2]
        adds r2, r2, #4

LoopFillZerobss:
        cmp r2, r4
        bcc FillZerobss

        /* ---------------------- CMSIS Startup Init -------------------------------------- */
        @ Call and return from our equivalents for the CMSIS SystemInit function.
        @ @@@ bl      MCUCORE_CLOCK_StartupInit
        @ @@@ bl      MCUCORE_SCB_MaybeEnableFPU
        @ @@@ bl      MCUCORE_VTOR_SetVectorTableLocation(void)

        /* Call static constructors */
        bl      __libc_init_array

        /*
        --------------------------------------------------------------------------------+-
        Call Application Main()

        At this point, the "system" must be in a standards-conforming state.
        This means that all the pre-conditions for a C program have been satisfied
        from the perspective of the C Language Standard.
        --------------------------------------------------------------------------------+-
        */
        bl      main              @ Call the application entry point. Appears to be no setup for argc and argv.

        /*
        --------------------------------------------------------------------------------+-
        Loop Forever Just In Case Main Returns

        This is fine if we have configured a watchdog timer.
        Otherwise we should take some action here to alert and recover the system.
        --------------------------------------------------------------------------------+-
        */
LoopForever:
        b       LoopForever       @ just in case main returns.

.size   Reset_Handler, . - Reset_Handler


