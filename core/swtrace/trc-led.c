
/*
================================================================================================#=
SOFTWARE TRACE LED

This module provides some handy services for blinking LEDs.
This file implements the module initialization capability.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "core/swtrace/trc-led.h"



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Initialize the GPIO peripherals and pins for the trace LEDs.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void TRC_External_LED_Init(void)
{
    // -------------------------------------------------------------+-
    // Enable the clock to the GPIO peripheral;
    // ToDo: abstract this and make it part of the mapping?
    // -------------------------------------------------------------+-
#if defined(MCUFAM_STM32F0)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
#elif defined(MCUFAM_STM32L4)
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
#else
    #error "MCU Family Name Not Defined."
#endif

    // -------------------------------------------------------------+-
    // Configure each pin for output.
    // -------------------------------------------------------------+-
    LL_GPIO_SetPinMode(
        SW_DEBUG_EXTERNAL_RED_LED_PERIPH,
        SW_DEBUG_EXTERNAL_RED_LED_PIN,
        LL_GPIO_MODE_OUTPUT
    );

    LL_GPIO_SetPinMode(
        SW_DEBUG_EXTERNAL_GREEN_LED_PERIPH,
        SW_DEBUG_EXTERNAL_GREEN_LED_PIN,
        LL_GPIO_MODE_OUTPUT
    );

    LL_GPIO_SetPinMode(
        SW_DEBUG_EXTERNAL_BLUE_LED_PERIPH,
        SW_DEBUG_EXTERNAL_BLUE_LED_PIN,
        LL_GPIO_MODE_OUTPUT
    );

    LL_GPIO_SetPinMode(
        SW_DEBUG_EXTERNAL_YELLOW_LED_PERIPH,
        SW_DEBUG_EXTERNAL_YELLOW_LED_PIN,
        LL_GPIO_MODE_OUTPUT
    );

    // Ensure each is turned off at initialization time;
    Trace_Red_Off();
    Trace_Green_Off();
    Trace_Blue_Off();
    Trace_Yellow_Off();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void TRC_OnBoard_LED_Init(void)
{
    // -------------------------------------------------------------+-
    // Enable AHB2 Clock to GPIO Port A in the Reset and Clock Control peripheral;
    // ToDo: abstract this and make it part of the mapping?
    // -------------------------------------------------------------+-
#if defined(MCUFAM_STM32F0)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
#elif defined(MCUFAM_STM32L4)
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
#else
    #error "MCU Family Name Not Defined."
#endif

    // Configure pin for output push-pull mode to drive the LED.
    LL_GPIO_SetPinMode(
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PERIPH,
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PIN,
        LL_GPIO_MODE_OUTPUT
    );

    // Ensure the LED is turned off at initialization time;
    Trace_OnBrdGreen_Off();
}



