
/*
================================================================================================#=
QUICK START BOARD MODEL
qs-board-model-nucleo-l476rg.h

This file defines a conceptual model of the capabilities provided by
an embedded target board and then maps each of those capabilities to
a specific hardware resource on the MCU.

The hardware capabilities are defined in terms of lines on the schematic.
The lines are selected and named within the context of a particular application
that depends on the capabilities realized by those lines.
The capabilities and line names should remain constant
across various models and versions of the board hardware.

Given this mental model, this file then maps the line names to
the MCU pins and peripherals to which those lines are connected
on a particular board revision, in this case, the NUCLEO-F091RC board.

BACKGROUND:
Whenever possible, the project code should be written in terms of the line names.
The schematic, and the line names on that schematic, should create a mental model
for the embedded developer that they reference and carry with them
as they create and maintain the project source code.

When the line names are chosen well, they will convey the semantic information
that a developer needs to work efficiently without having to constantly reference the schematic.

Note: The focus of this mapping is on the MCU pins and peripherals.
Changing this mapping to a different pin or peripheral could have impacts well beyond
just the pin number and peripheral. Those impacts may require additional code and/or
some conditional compilation based on the board type.

DEPENDENCIES:
* Definitions from the STM32 Low Level Driver
* UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
* NUCLEO-L476RG target board

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#pragma once

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_gpio.h"


// =============================================================================================#=
// Public Definitions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Each GPIO Pin and Peripheral are defined as follows:
//
// <schematic_line_name>_PIN
//     the pin to which the line is connected;
//
// <schematic_line_name>_PERIPH
//     the internal MCU peripheral that services the pin;
//
// Note: since we don't have a schematic for the NUCLEO board,
// we invent our own meaningful line names.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -------------------------------------------------------------+-
// Software Debug External Off-Board LEDs.
//
// We choose to use these GPIOs: PC0, PC1, PC2, PC3;
//
// We know from the UM1724 user manual,
// that PC0-3 are connected to CN7 Morpho Connector pins 35-38;
// See Figure 13 on page 29 and Table 32 on page 61;
// -------------------------------------------------------------+-
#define  SW_DEBUG_EXTERNAL_RED_LED_PERIPH     GPIOC
#define  SW_DEBUG_EXTERNAL_RED_LED_PIN        LL_GPIO_PIN_0

#define  SW_DEBUG_EXTERNAL_GREEN_LED_PERIPH   GPIOC
#define  SW_DEBUG_EXTERNAL_GREEN_LED_PIN      LL_GPIO_PIN_1

#define  SW_DEBUG_EXTERNAL_BLUE_LED_PERIPH    GPIOC
#define  SW_DEBUG_EXTERNAL_BLUE_LED_PIN       LL_GPIO_PIN_2

#define  SW_DEBUG_EXTERNAL_YELLOW_LED_PERIPH  GPIOC
#define  SW_DEBUG_EXTERNAL_YELLOW_LED_PIN     LL_GPIO_PIN_3


// -------------------------------------------------------------+-
// Software Debug On-Board Green LED.
//
// We choose to use the On-Board Green User LED (LD2).
// See Figure 3 on page 13 of the UM1724 user manual.
//
// We know from Section 6.4 on page 23,
// that User LED (LD2) is connected to 'Arduino' Pin D13
// which corresponds to GPIO PA5 on the MCU.
//
// See also Figure 13 on page 29 and Table 11 on page 38;
// Pin D13 is available on Arduino CN5 Digital pin 6.
// -------------------------------------------------------------+-
#define  SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PERIPH    GPIOA
#define  SW_DEBUG_EXTERNAL_OnBrdGreen_LED_PIN       LL_GPIO_PIN_5


// -----------------------------------------------------------------------------+-
// On-Board User Button (B1)
//
// We choose to use the on-board B1 User Button.
// See Figure 3 on page 13 of the UM1724 user manual.
//
// We know from Section 6.5 on page 23,
// that the B1 user button is connected PC13;
// -----------------------------------------------------------------------------+-
#define  ON_BOARD_USER_BUTTON_PERIPH   GPIOC
#define  ON_BOARD_USER_BUTTON_PIN      LL_GPIO_PIN_13


