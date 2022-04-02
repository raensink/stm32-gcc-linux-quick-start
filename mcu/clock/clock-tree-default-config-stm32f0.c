
/*
================================================================================================#=
CLOCK TREE DEFAULT CONFIG
mcu/clock/clock-tree-default-config-stm32f0.c

Provides a default configuration service for clock tree,
including the Cortex-M System Timer (SysTick).

DEPENDENCIES:
    STM32 Cube HAL Low Level Drivers;
    STM32F0 MCU;

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "mcu/clock/clock-tree-default-config.h"

#include "CMSIS/Device/ST/STM32F0xx/Include/stm32f091xc.h"

// STM32 Low Level Drivers
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_rcc.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_utils.h"


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
void mcu_clock_tree_default_config(void)
{
    // Configure the PLL to (HSI / 2) * 12 = 48MHz.
    // Use a PLLMUL of 0xA for *12, and keep PLLSRC at 0
    // to use (HSI / PREDIV) as the core source. HSI = 8MHz.
    RCC->CFGR  &= ~(RCC_CFGR_PLLMUL |
                    RCC_CFGR_PLLSRC);
    RCC->CFGR  |=  (RCC_CFGR_PLLSRC_HSI_DIV2 |
                    RCC_CFGR_PLLMUL12);
    // Turn the PLL on and wait for it to be ready.
    RCC->CR    |=  (RCC_CR_PLLON);
    while (!(RCC->CR & RCC_CR_PLLRDY)) {};
    // Select the PLL as the system clock source.
    RCC->CFGR  &= ~(RCC_CFGR_SW);
    RCC->CFGR  |=  (RCC_CFGR_SW_PLL);
    while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) {};

    // Set the global clock frequency variable.
    HCLK_Frequency_Hz = 48000000;

    // ?????? TODO ??????
    // What's going on here?
    // Configure the Cortex-M SysTick source for 1000 ticks per second given the HCLK frequency;
    // Sets RELOAD register value to (HCLKFrequency / TicksPerSecond) - 1;
    // Clears the counter value;
    // Sets CSR:Clock source to use processor clock rather than external clock;
    // Leaves CSR:TickInt unchanged (zero?) so assume no SysTick exception? But then why set it?
    // Sets CSR:Enable to enable the counter
    // TicksPerSecond = 1000U;
    LL_InitTick(HCLK_Frequency_Hz, TicksPerSecond);

    // Set the CMSIS SystemCoreClock variable to record the freq of the HCLK;
    LL_SetSystemCoreClock(HCLK_Frequency_Hz);
};

