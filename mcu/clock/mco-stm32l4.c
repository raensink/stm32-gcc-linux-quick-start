
/*
================================================================================================#=
Microcontroller Clock Output (MCO)
mcu/clock/mco-stm32f0.c

Note: according to Figure 24 on page 35 of UM1724
STM32 Nucleo-64 Boards User Manual, PA8 is available on CN10 Pin #23.

DEPENDENCIES:
    STM32F0 HAL Low Level Drivers

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "mcu/clock/mco.h"

// MCU Device Definition
#include "CMSIS/Device/ST/STM32L4xx/Include/stm32l476xx.h"

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_bus.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_rcc.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_gpio.h"


// =============================================================================================#=
// External API Services
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Configure Microcontroller Clock Output (MCO)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void MCU_Clock_MCO_Config(void)
{
    // Configure GPIO PIN
    //
    // Based on Table 16 on page 84 of the STM32L476xx Data Sheet, we know that the MCO is on PIN PA8.
    // Enable the peripheral bus clock to the GPIOA peripheral which is on the AHB2 bus
    // and configure the pin accordingly for MCO.
    LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(       GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(    GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_0);
    LL_GPIO_SetPinOutputType( GPIOA, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(       GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinSpeed(      GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_VERY_HIGH);

    // Configure the MCO source selection and divider.
    // Note: use the MCO prescaler divider as needed to ensure that
    // the output signal can by your Saleae or other instrument.
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_SYSCLK, LL_RCC_MCO1_DIV_8);

    // Some handy MCO alternative configurations:
    // LL_RCC_ConfigMCO( LL_RCC_MCO1SOURCE_MSI, LL_RCC_MCO1_DIV_1);
    // LL_RCC_ConfigMCO( LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_1);
    // LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_SYSCLK, LL_RCC_MCO1_DIV_8);
};


