
/*
================================================================================================#=
CLOCK TREE DEFAULT CONFIG
mcu/clock/clock-tree-default-config-stm32f0.c

Provides a default configuration service for clock tree,
including the Cortex-M System Timer (SysTick).

The purpose of this software module is to perform the clock startup initialization services.
At this time there is only one such service and that is to reset the clock configuration.
Perhaps in the future we will have more clock related services and locate those here as well.

This module contains a project-specific adaptation of the CMSIS SystemInit() function.
It is based on the system_<device>.c file from the STM32CubeL4 MCU Package:
    Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c

According to the CMSIS documentation, the CMSIS system file
may require application specific adaptations and therefore
should be copied into the application project folder.
We have done this, in part, by refactoring the clock configuration code and putting it into this new module.

DEPENDENCIES:
    STM32 Cube HAL Low Level Drivers;
    STM32F0 MCU;

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "mcu/clock/clock-tree-default-config.h"

#include "CMSIS/Device/ST/STM32L4xx/Include/stm32l4xx.h"

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_rcc.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_utils.h"


// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// -----------------------------------------------------+-
// Frequency of the HCLK in Hz;
// Set by the clock tree configuration code below;
// TODO: REFACTOR - move to cmsis-clock?
// -----------------------------------------------------+-
static uint32_t HCLK_Frequency_Hz;

// -----------------------------------------------------+-
// Frequency of the SysTick Timer in ticks-per-second;
// Set by the clock tree configuration code below;
// @@@ or is it really??? @@@
// TODO: REFACTOR
// TODO: REFACTOR - move to cmsis-clock?
// -----------------------------------------------------+-
static uint32_t TicksPerSecond;



// =============================================================================================#=
// Public API Services
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// Clock Tree Default Configuration
// -----------------------------------------------------------------------------+-

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
MCUCORE_CLOCK_StartupInit(void)

The goal here should be: configure the clock just enough to ensure
that we can run the rest of the startup code.

The MCU has three internal oscillators:
    LSI - Low Speed Internal
    HSI - High Speed Internal
    MSI - Multi-Speed Internal

Three different clock sources can be used to drive SYSCLK
• HSI (High speed Internal) oscillator clock
• HSE (High speed External) oscillator clock
• PLL clock

The MCU has two secondary clock sources
    • 32 kHz low speed internal (LSI) RC
        for the independent watchdog and to optionally drive RTC

    • 32.768 kHz low speed external crystal (LSE)
        to optionally drive RTC

Each clock source can be switched on/off independently
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
void MCU_Clock_Tree_Default_Config(void)
{
    // -----------------------------------------------------------------------------+-
    // You must have a picture of the clock tree in front of you
    // if you ever expect to understand this code.
    // See Figure 15 on page 208 of the RM0351 Reference Manual.
    //
    // HSE     - High Speed External
    // MSI     - Multi-Speed Internal Clock
    // SYSCLK  - System Clock
    // HCLK    - AHB clock, a.k.a. CPU Clock.
    //           HCLK is the main CPU clock, also used for the AHB.
    //           This is often referred to as "The AHB clock (HCLK)" in the reference manual.
    //           It can be gated when the CPU is sleeping (WFI for example)
    // PCLK1   - APB1 Clock
    // PCLK2   - APB2 Clock
    //
    // SysTick - Cortex System Timer
    //           SysTick is the ARMv7-M standard "system tick" timer commonly used as
    //           a timebase in real-time operating systems.
    //
    // RCC - Reset and Clock Control
    // CR  - Clock Control Register
    // MCO - Microcontroller Clock Output
    //       This pin outputs one of the internal clocks for external use.
    // CSS - Clock Security System
    //
    // -----------------------------------------------------------------------------+-
    // Upon completion of this function, the clocks are configured like so:
    //     SYSCLK source = MSI          (default reset value)
    //     AHB Prescaler = divide by 1  (default reset value)
    //     MSI:    4MHz (default)       (default reset value)
    // SO:
    //     SYSCLK: 4MHz
    //     HCLK:   4MHz
    // -----------------------------------------------------------------------------+-

    // Turn on the MSI Clock
    //
    // The MSI clock signal is generated from an internal RC oscillator.
    // Its frequency range can be adjusted by software by using bits in the Clock control register (RCC_CR).
    // After restart from Reset, the MSI frequency is set to its default value 4 MHz.
    // The MSI RC oscillator has the advantage of providing a low-cost (no external components)
    // low-power clock source. In addition, when used in PLL-mode with the LSE, it provides
    // a very accurate clock source which can be used by the USB OTG FS device,
    // and feed the main PLL to run the system at the maximum speed 80 MHz.
    //
    // The MSIRDY flag in the Clock control register (RCC_CR) indicates whether the MSI RC
    // is stable or not. At startup, the MSI RC output clock is not released until this bit is set by hardware.
    // The MSI RC can be switched on and off by using the MSION bit in the Clock control register (RCC_CR).
    RCC->CR |= RCC_CR_MSION;

    // Return the Clock Configuration Register to it's reset value.
    // RCC_CFGR reset value is 0x0000 0000
    //
    // MCO: divide by 1
    // MCO: output disabled
    // MSI: oscillator selected as wakeup from stop clock and CSS backup clock
    // APB2: Prescaler: divide by 1
    // APB1: Prescaler: divide by 1
    // AHB: Prescaler: SYSCLOCK divide by 1
    // System Clock Switch: MSI Oscillator used as the system clock.
    RCC->CFGR = 0x00000000U;

    // Reset and Clock Control - Clock Control Register
    // RCC_CR reset value is 0x0000 0063
    // Diable all other clocks
    // ---------------------------------
    // Bit  Meaning              Setting
    // ---------------------------------
    // 31 - Reserved             1: no change
    // 30 - Reserved             1: no change
    // 29 - PLL SAI2 RDY (RO)    1: no change
    // 28 - PLL SAI2 On          0: Reset/Off

    // 27 - PLL SAI1 RDY (RO)    1: no change
    // 26 - PLL SAI1 On          0: Reset/Off
    // 25 - Main PLL RDY (RO)    1: no change
    // 24 - Main PLL On          0: Reset/Off

    // 23 - Reserved             1: no change
    // 22 - Reserved             1: no change
    // 21 - Reserved             1: no change
    // 20 - Reserved             1: no change

    // 19 - CSS ON               0: Reset/Off
    // 18 - HSE BYP              1: no change (this is reset below)
    // 17 - HSE RDY (RO)         1: no change
    // 16 - HSE ON               0: Reset/Off

    // 15 - Reserved             1: no change
    // 14 - Reserved             1: no change
    // 13 - Reserved             1: no change
    // 12 - Reserved             1: no change

    // 11 - HSI ASFS             1: no change
    // 10 - HSI RDY (RO)         1: no change
    // 09 - HSI KERON            1: no change
    // 08 - HSION                1: no change

    // Default is 0x6: "around 4 MHz"
    // 07 - MSIRANGE             1: no change
    // 06 - MSIRANGE             1: no change
    // 05 - MSIRANGE             1: no change
    // 04 - MSIRANGE             1: no change

    // After a standby or a reset MSIRGSEL is at 0 which means
    // the MSI range value is provided by MSISRANGE in CSR register.
    // 03 - MSIRGSEL             1: no change
    // 02 - MSIPLLEN             1: no change, default is OFF
    // 01 - MSIRDY (RO)          1: no change
    // 00 - MSION                1: no change, default is ON, already set above.

    // Note: This is an AND operation and so
    // only the zeros will have any effect.
    // See above for details.
    RCC->CR &= 0xEAF6FFFFU;

    // PLL Configuration.
    // Reset value is 0x0000 1000
    // None of the PLLs are enabled.
    RCC->PLLCFGR = 0x00001000U;

    // Reset HSEBYP
    // Not sure why this wasn't done above.
    // Not sure why this matters since HSE is not enabled.
    RCC->CR &= 0xFFFBFFFFU;

    // Clock Interrupt Enable Register
    // Reset Value: 0x0000 0000
    // All such interrupts disabled.
    RCC->CIER = 0x00000000U;
	
	
	
}

