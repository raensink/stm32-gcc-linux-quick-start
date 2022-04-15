
/*
================================================================================================#=
CLOCK TREE DEFAULT CONFIG
mcu/clock/clock-tree-default-config.h

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#pragma once

#include <stdint.h>


// -----------------------------------------------------------------------------+-
// Set Clock Tree Default Configuration
// -----------------------------------------------------------------------------+-
void MCU_Clock_Tree_Default_Config(void);


// -----------------------------------------------------------------------------+-
// Getters for key clock-tree configuration values;
// -----------------------------------------------------------------------------+-
uint32_t MCU_Clock_Get_PCLK1_Frequency_Hz(void);
uint32_t MCU_Clock_Get_PCLK2_Frequency_Hz(void);

