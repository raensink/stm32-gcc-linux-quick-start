
/*
================================================================================================#=
MCU Core System Control Block

The purpose of this module is to provide SCB startup initialization services.
At this time there is only one such service and that is to enable the coprocessor.

This module contains a project-specific adaptation of the CMSIS SystemInit() function.
It is based on the system_<device>.c file from the STM32CubeL4 MCU Package:
    Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c

According to the CMSIS documentation, the CMSIS system file
may require application specific adaptations and therefore
should be copied into the application project folder.
We have done this, in part, by refactoring the coprocessor configuration code and putting it into this new module.
================================================================================================#=
*/

#include "core/scb.h"
#include "stm32l4xx.h"


/*
--------------------------------------------------------------------------------+-
Enable Co-Processor (if configured)

Based on the build-time configuration, enable the coprocessor via the SCB.
This should be called from the reset handler.

The CMSIS header files provide a C-level abstraction of the underlying Cortex-M4 core.
The CMSIS-Core header files use two C macros:
    __FPU_PRESENT  :  Defined if FPU is present
    __FPU_USED     :  Defined if FPU is used in this application.

When both are set, the system initialization function, SystemInit(),
will enable the FPU.  The FPU must be enabled before any FPU instruction is executed,
otherwise a hardware exception will be raised.

If your application does not need to handle any floating point calculations,
then you can leave the FPU switched off all the time by not defining the __FPU_USED macro.
This can reduce power consumption.

    __FPU_PRESENT is defined by stm32l476xx.h or its equivalent

    __FPU_USED  is defined in CMSIS/Core/Include/core_cm4.h
        based on __GNUC__, __VFP_FP__, and/or other build-time
        configuration symbols.

Quoted From:
    Joseph Yiu, in The Definitive Guide to ARM® CORTEX®-M3 and CORTEX®-M4 Processors (Third Edition), 2014

13.2.3 CPACR register
The CPACR register allows you to enable or disable the FPU.
It is located in address 0xE000ED88 and can be accessed as “SCB->CPACR” in CMSIS-Core.
Bit 0 to bit 19 and bit 24 to bit 31 are not implemented and are reserved.
This programmer’s model of this register provides enable control for up to 16 co-processors.
On the Cortex®-M4, the FPU is defined as co-processor 10 and 11.
Since there is no other co-processor, only CP10 and CP11 are available and both are for the FPU.
When programming this register, the settings for CP10 and CP11 must be identical.
The encoding for CP10 and CP11 is shown in Table 13.3.

Table 13.3. CP10 and CP11 Settings
Bits   CP10 and CP11 Setting
00     Access denied. Any attempted access generate a Usage fault (type NOCP – No Co-processor)
01     Privileged Access only. Unprivileged access generate a Usage fault
10     Reserved – result unpredictable
11     Full access

By default CP10 and CP11 are zero after reset.
This setting disables the FPU and allows lower power consumption.
Before using FPU, you need to program the CPACR to enable the FPU first. For example:

    SCB->CPACR|= 0x00F00000; // Enable the floating point unit for full access

This step is typically carried out inside the SystemInit() function provided
in the device-specific software package file. SystemInit() is executed by the reset handler.
--------------------------------------------------------------------------------+-
*/
void MCUCORE_SCB_MaybeEnableFPU(void)
{
#if defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)
#if defined (__FPU_USED) && (__FPU_USED == 1U)

    // Set CP10 and CP11 for Full Access
    SCB->CPACR |= ((3UL << 20U)|(3UL << 22U)); 

#endif
#endif

    return;
}


