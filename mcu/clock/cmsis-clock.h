
/*
================================================================================================#=
CMSIS Clock

The purpose of this module is to satisfy a small number
of clock-related dependencies that are provided by standard CMSIS code.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#pragma once

#include <stdint.h>

extern uint32_t  SystemCoreClock;

extern const uint8_t  AHBPrescTable[];


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Use the given value to set the SystemCoreClock variable.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SystemCoreClock_record_global_hclk_freq(uint32_t freq_in_hz);


