
/*
================================================================================================#=
SW Trace LED

This module provides some handy services for blinking LEDs.
These may be invoked at will in various locations in the code
when other means for execution trace are not yet available.
Be sure to call the initialization function first.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#pragma once

#include "qs-board-model-nucleo-f091rc.h"


// =============================================================================================#=
// Public API Services
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Three functions: on, off, and toggle
// for each of the four external LEDs: red, green, blue, and yellow.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
#define Trace_Red_On() do { \
    LL_GPIO_SetOutputPin(   \
        SW_DEBUG_EXTERNAL_RED_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_RED_LED_PIN     \
    ); \
} while(0)

#define Trace_Red_Off() do { \
    LL_GPIO_ResetOutputPin(  \
        SW_DEBUG_EXTERNAL_RED_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_RED_LED_PIN     \
    ); \
} while(0)

#define Trace_Red_Toggle() do { \
    LL_GPIO_TogglePin(          \
        SW_DEBUG_EXTERNAL_RED_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_RED_LED_PIN     \
    ); \
} while(0)


// -----------------------------------------------------------------------------+-
#define Trace_Green_On() do { \
    LL_GPIO_SetOutputPin(     \
        SW_DEBUG_EXTERNAL_GREEN_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_GREEN_LED_PIN     \
    ); \
} while(0)

#define Trace_Green_Off() do { \
    LL_GPIO_ResetOutputPin(    \
        SW_DEBUG_EXTERNAL_GREEN_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_GREEN_LED_PIN     \
    ); \
} while(0)

#define Trace_Green_Toggle() do { \
    LL_GPIO_TogglePin(            \
        SW_DEBUG_EXTERNAL_GREEN_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_GREEN_LED_PIN     \
    ); \
} while(0)


// -----------------------------------------------------------------------------+-
#define Trace_Blue_On() do { \
    LL_GPIO_SetOutputPin(    \
        SW_DEBUG_EXTERNAL_BLUE_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_BLUE_LED_PIN     \
    ); \
} while(0)

#define Trace_Blue_Off() do { \
    LL_GPIO_ResetOutputPin(   \
        SW_DEBUG_EXTERNAL_BLUE_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_BLUE_LED_PIN     \
    ); \
} while(0)

#define Trace_Blue_Toggle() do { \
    LL_GPIO_TogglePin(           \
        SW_DEBUG_EXTERNAL_BLUE_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_BLUE_LED_PIN     \
    ); \
} while(0)


// -----------------------------------------------------------------------------+-
#define Trace_Yellow_On() do { \
    LL_GPIO_SetOutputPin(      \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PIN     \
    ); \
} while(0)

#define Trace_Yellow_Off() do { \
    LL_GPIO_ResetOutputPin(     \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PIN     \
    ); \
} while(0)

#define Trace_Yellow_Toggle() do { \
    LL_GPIO_TogglePin(             \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_YELLOW_LED_PIN     \
    ); \
} while(0)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Three functions: on, off, and toggle
// for the one on-board LED: green.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
#define Trace_OnBrdGreen_On() do { \
    LL_GPIO_SetOutputPin(                      \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PIN     \
    ); \
} while(0)

#define Trace_OnBrdGreen_Off() do { \
    LL_GPIO_ResetOutputPin(         \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PIN     \
    ); \
} while(0)

#define Trace_OnBrdGreen_Toggle() do { \
    LL_GPIO_TogglePin(                 \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PERIPH, \
        SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PIN     \
    ); \
} while(0)



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Do the needful for one-time startup initialization;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SW_Trace_External_LED_Init(void);
void SW_Trace_OnBoard_LED_Init(void);


