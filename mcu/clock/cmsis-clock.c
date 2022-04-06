
/*
================================================================================================#=
CMSIS Clock

The purpose of this module is to satisfy a small number
of clock-related dependencies that are provided by standard CMSIS code.
It is common to find example code and templates out in the STM32 ecosystem
that have dependencies on the data defined here.

This cmsis-clock module is a project-specific adaptation of the example CMSIS system file.
According to the CMSIS documentation, the CMSIS system file may require application specific
adaptations and therefore should be copied into the application project folder.
That is exactly what we have done here, at least in part.

This module is based on the following files from the various STM32Cube MCU packages:

    Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/system_stm32f0xx.c
    Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "mcu/clock/cmsis-clock.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// GLOBAL: SystemCoreClock
//
// This global variable should be set to the frequency of the HCLK in Hz;
//
// This variable is specified by and required by the CMSIS.
// It is used by the STM32 HAL Low Level Drivers and, quite likely,
// other source code within the STM32 ecosystem as well.
//
// The definition provided in the CMSIS documentation is sorely lacking.
// After some digging around in the LL drivers, one can find
// the following function which does add some clarity:
//
//     // This function sets directly SystemCoreClock CMSIS variable.
//     // HCLKFrequency - HCLK frequency in Hz
//     void LL_SetSystemCoreClock(uint32_t HCLKFrequency)
//     {
//         SystemCoreClock = HCLKFrequency;
//     }
//
// Also note that the CMSIS SystemCoreClockUpdate() function makes an attempt to
// derive the correct value for the SystemCoreClock variable by looking at
// the current state of the hardware.  We choose to avoid doing this
// as much as possible and prefer rather to just set it to a known value directly.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
uint32_t SystemCoreClock;


/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
MSIRangeTable[]
    List of the available frequencies for the STM32L4 MSI clock.

MSI Multi-Speed Internal RC Oscillator Clock
    According to the RM0351 STM32L4 Reference Manual:
    "The MSI clock signal is generated from an internal RC oscillator.
    Its frequency range can be adjusted by software by using the MSIRANGE[3:0] bits
    in the Clock control register (RCC_CR).
    Twelve frequency ranges are available:
        100 kHz, 200 kHz, 400 kHz, 800 kHz,
        1 MHz, 2 MHz, 4 MHz (default value), 8 MHz,
        16 MHz, 24 MHz, 32 MHz, and 48 MHz.

    The MSI clock is used as system clock after Restart from Reset,
    Wakeup from Standby, and Shutdown low-power modes.
    After restart from Reset, the MSI frequency is set to its default value 4 MHz."
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
#if defined(MCUFAM_STM32L4)
const uint32_t MSIRangeTable[12] = {
      100000U,
      200000U,
      400000U,
      800000U,

     1000000U,
     2000000U,
     4000000U,
     8000000U,

    16000000U,
    24000000U,
    32000000U,
    48000000U
};
#endif



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// GLOBAL: AHBPrescTable[]
//
// This is a list of prescalar values defined for the HPRE field in the RCC CFGR register;
// The idea is that one can take the value of the HPRE field, and use it as an index into
// this AHB Prescalar Table to determine by how many bits the SYSCLK frequency value
// should be shifted right (divided) to get the HCLK value.
// High marks for cleverness, not so much for developer experience.
//
// TODO: add reference to the device clock tree here.
// For example, according to the RM0091 STM32F0 Reference Manual:
//     RCC_CFGR: HPRE[3:0]: AHB prescaler
//         0xxx: SYSCLK not divided
//         1000: SYSCLK divided by 2
//         1001: SYSCLK divided by 4
//         1010: SYSCLK divided by 8
//         1011: SYSCLK divided by 16
//         1100: SYSCLK divided by 64
//         1101: SYSCLK divided by 128
//         1110: SYSCLK divided by 256
//         1111: SYSCLK divided by 512
//
// This AHB Prescalar Table is used by __LL_RCC_CALC_HCLK_FREQ() HAL function.
// (That macro is great example of how NOT to write excellent code.)
// This table is also used in SystemCoreClockUpdate() which we choose to avoid.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

const uint8_t  AHBPrescTable[16] = {
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Use the given value to set the SystemCoreClock variable.
//
// This does not actually change the value of the HCLK in the hardware.
// It only records the value so other code can access it directly.
// This is not a practice we would recommend, but such is CMSIS.
//
// A better approach would be:
//     Make SystemCoreClock a private variable;
//     Use accessor functions (such as this one) to get and set the value;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SystemCoreClock_record_global_hclk_freq(uint32_t freq_in_hz)
{
    SystemCoreClock = freq_in_hz;
}


