
/*
================================================================================================#=
CLOCK TREE DEFAULT CONFIG
mcu/clock/clock-tree-default-config-stm32l4.c

Provides a default configuration service for
the clock tree including the Cortex-M System Timer (SysTick).

The purpose of this software module is to perform startup initialization on the clock tree.
At this time there is only one such service.  Perhaps in the future we will have more
clock-related services and locate those here as well.

This module contains a project-specific adaptation of the CMSIS SystemInit() function.
It is very roughly based on the system_<device>.c file from the STM32 Cube MCU Package
and various STM32 example code.

According to the CMSIS documentation, the CMSIS system file may require
application specific adaptations and therefore should be copied into
the application project folder.  That is what we have done here, at least in part.

DEPENDENCIES:
    STM32 Cube HAL Low Level Drivers;
    STM32L4 MCU;

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
// Frequency of the APB1 PCLK1 in Hz;
// Frequency of the APB2 PCLK1 in Hz;
// Set by the clock tree configuration code below;
// TODO: REFACTOR - move to cmsis-clock?
// -----------------------------------------------------+-
static uint32_t PCLK1_Frequency_Hz;
static uint32_t PCLK2_Frequency_Hz;

// -----------------------------------------------------+-
// Frequency of the SysTick Timer in ticks-per-second;
// Set by the clock tree configuration code below;
// @@@ Perhaps move this to CMSIS-Clock?
// -----------------------------------------------------+-
static uint32_t SysTick_Frequency_Hz;



// =============================================================================================#=
// Public API Services
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// Clock Tree Default Configuration
// -----------------------------------------------------------------------------+-

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
MCU Clock-Tree Default Config()

BACKGROUND
You must have a picture of the clock tree in front of you if you expect to understand this code.
See Figure 15 on page 208 of the RM0351 Reference Manual.

The L4 MCU has four clock sources that can be used to drive the system clock:
• HSI (High speed Internal) oscillator clock.
• MSI (Multi-Speed Internal) oscillator clock.
• HSE (High speed External) oscillator clock.
• PLL clock.
Each clock source can be switched on/off independently

Additional Concepts and Terminology:
SYSCLK  - System Clock

HCLK    - AHB clock, a.k.a. CPU Clock.
          HCLK is the main CPU clock, also used for the AHB.
          This is often referred to as "The AHB clock (HCLK)" in the reference manual.
          It can be gated when the CPU is sleeping (WFI for example)

PCLK1   - APB1 Clock
PCLK2   - APB2 Clock

SysTick - Cortex System Timer
          SysTick is the ARMv7-M standard "system tick" timer commonly used as
          a timebase in real-time operating systems.

RCC  - Reset and Clock Control

CR   - Clock Control Register

MCO  - Microcontroller Clock Output
       This pin outputs one of the internal clocks for external use.

----------------------------------------------------------------+-
CLOCK-TREE CONFIGURATION
----------------------------------------------------------------+-
Upon completion, this function configures the tree like so:
    MSI(4MHz) ==> PLL
    PLLCLK:       80MHz
    SYSCLK:       80MHz
    HCLK:         80MHz
    PCLK1:        80MHz
    PCLK2:        80MHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
void MCU_Clock_Tree_Default_Config(void)
{
    // ---------------------------------------------------------------------+-
    // Enable the MSI and allow it to run at its default 4MHz rate.
    //
    // The MSI clock signal is generated from an internal RC oscillator.
    // Its frequency range can be adjusted using bits in the RCC_CR.
    // After restart from Reset, the MSI frequency is set to its default value of 4 MHz.
    // The MSI RC oscillator has the advantage of providing a low-cost (no external components)
    // low-power clock source. In addition, when used in PLL-mode, it provides a very accurate
    // clock source which can be used to run the system at the maximum speed 80 MHz.
    //
    // The MSIRDY flag in the Clock control register (RCC_CR) indicates whether
    // the MSI RC is stable. At startup, the MSI RC output clock is not released
    // until this bit is set by hardware.  The MSI RC can be switched on and off
    // by using the MSION bit in the clock control register (RCC_CR).
    // ---------------------------------------------------------------------+-
    LL_RCC_MSI_Enable();
    while(1 != LL_RCC_MSI_IsReady()) {};

    // ---------------------------------------------------------------------+-
    // Configure and enable the PLL as follows:
    //
    //     Input: MSI
    //     PLLM:    1
    //     PLLN:   40
    //     PLLR:    2
    //
    // This yields: PLLCLK = (4MHz * (40/1)) / 2 = 80MHz
    // ---------------------------------------------------------------------+-
    uint32_t plln = 40;
    LL_RCC_PLL_ConfigDomain_SYS(
        LL_RCC_PLLSOURCE_MSI,
        LL_RCC_PLLM_DIV_1,
        plln,
        LL_RCC_PLLR_DIV_2
    );

    // Set PLLON in RCC Clock Control register
    LL_RCC_PLL_Enable();

    // ---------------------------------------------------------------------+-
    // Enable the PLLR output "mapped on SYSCLK domain";
    // Set PLLREN Main PLLCLK Output Enable;
    // And wait until PLLRDY is true in RCC Clock Control register
    // ---------------------------------------------------------------------+-
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1) {};

    // ---------------------------------------------------------------------+-
    // Set System Clock Source to PLL
    // SYSCLK = PLLCLK = 80MHz
    // Wait until System Clock Switch Status indicates PLL is the source.
    // ---------------------------------------------------------------------+-
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {};

    // ---------------------------------------------------------------------+-
    // Set AHB Prescaler to 1;
    // HCLK = SYSCLK(80MHz)/1;
    // Record the freq of the HCLK to keep the CMSIS dependencies happy;
    // ---------------------------------------------------------------------+-
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    HCLK_Frequency_Hz = 80000000UL;
    LL_SetSystemCoreClock(HCLK_Frequency_Hz);

    // ---------------------------------------------------------------------+-
    // Set APB1 and APB2 prescalers to HCLK(80MHz) / 1
    // ---------------------------------------------------------------------+-
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    PCLK1_Frequency_Hz = HCLK_Frequency_Hz;

    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    PCLK2_Frequency_Hz = HCLK_Frequency_Hz;

    // -----------------------------------------------------------------------------+-
    // Configure the Cortex-M SysTick source for 1msec tick.
    // -----------------------------------------------------------------------------+-
    // SysTick provides a simple, 24-bit, decrementing, wrap-on-zero counter;
    // If enabled, counting down to zero will cause the SysTick exception to be pended;
    //
    // The counter Clock Source can be one of the following two choices:
    //     HCLK/1 (CLKSOURCE Bit = 1)
    //     HCLK/8 (CLKSOURCE Bit = 0)
    //
    // HCLK is set for 80Mhz (see above);
    // We configure the SysTick counter to be
    // driven at this clock freq directly.
    //
    // We desire a 1kHz SysTick frequency:
    // Equation:    80Mhz/ReloadValue = 1kHz
    // Solving:     ReloadValue = 80Mhz/1kHz
    // Generalize:  ReloadValue = HCLK_Frequency_Hz / SysTick_Frequency_Hz
    // We subtract one to account for zero based counting;
    // -----------------------------------------------------------------------------+-
    SysTick_Frequency_Hz = 1000UL; // 1 tick every 1 milli-second;
    SysTick->LOAD  = (uint32_t)((HCLK_Frequency_Hz / SysTick_Frequency_Hz) - 1UL);

    // Load the initial SysTick Counter Value;
    SysTick->VAL   = 0UL;

    // Set Priority for SysTick Interrupt
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

    // CLKSOURCE   - Clock Source set as HCLK/1
    // TICKINT     - Enable the SysTick Exception
    // ENABLE      - Enable the SysTick Counter
    SysTick->CTRL  = 0UL;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
};


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
uint32_t MCU_Clock_Get_PCLK1_Frequency_Hz(void)
{
    return PCLK1_Frequency_Hz;
};

// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
uint32_t MCU_Clock_Get_PCLK2_Frequency_Hz(void)
{
    return PCLK2_Frequency_Hz;
};


#if 0
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
Source Material - alternate implementation
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@

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
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
#endif
