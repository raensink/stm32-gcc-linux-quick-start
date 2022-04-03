
/*
================================================================================================#=
SOFTWARE TRACE LED


SPDX-License-Identifier: MIT-0

TODO: @@@@@@@@@@@@ Add Pin-Periph-LL Mapping @@@@@@@@@@@@@@
================================================================================================#=
*/

#include "core/swtrace/swtrace-led.h"

// STM32 Low Level Drivers
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_bus.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_gpio.h"


// -----------------------------------------------------------------------------+-
// Initialize the GPIO peripherals and pins for the trace LEDs.
// -----------------------------------------------------------------------------+-
void SW_Trace_LED_Init(void)
{
    // -----------------------------------------------------------------------------+-
    // GPIO PA5;
    // On-Board Green User LED (LD2)
    //
    // We know from the UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
    // that the on-board green user LED (LD2) is connected to 'Arduino' Pin D13;
    // See Figure 3 and Section 6.4;
    //
    // We also know from Table 23 on page 52
    // that D13 is connected to GPIO PA5 on the MCU;
    // -----------------------------------------------------------------------------+-

    // Enable AHB2 Clock to GPIO Port A in the Reset and Clock Control peripheral;
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    // Configure pin for output push-pull mode to drive the LED
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT); // On-Board LD2

    // -----------------------------------------------------------------------------+-
    // GPIO PC0, PC1, PC2, PC3;
    // External Off-Board LEDs
    //
    // We know from the UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
    // that PC0-3 are connected to Morpho connector pins 35-38;
    // See Table 32 on page 61;
    // -----------------------------------------------------------------------------+-
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT); // Red LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT); // Green LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT); // Blue LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT); // Yellow LED

    // Turn them all off for starters;
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_2);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_3);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
}


