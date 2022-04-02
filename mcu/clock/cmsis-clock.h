
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

// =============================================================================================#=
// Global Public Data
//
// Yes, we know this is bad practice but... CMSIS forces us to do so.
// =============================================================================================#=
extern uint32_t  SystemCoreClock;

extern const uint8_t  AHBPrescTable[];


// =============================================================================================#=
// Public API Services
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Use the given value to set the global SystemCoreClock variable.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SystemCoreClock_record_global_hclk_freq(uint32_t freq_in_hz);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Get the HCLK frequency as recorded in the global SystemCoreClock variable.
// Returns frequence in HZ.
//
// TODO: not yet implemented;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// uint32_t SystemCoreClock_get_hclk_freq(void);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Use the given value to record the number of ticks-per-second
// that the Cortex-M System Timer (SysTick) has been configured for.
// TODO: not yet implemented;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// void SysTick_record_ticks_per_second(uint32_t ticks_per_second);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Get the SysTick HCLK frequency as recorded in the global SystemCoreClock variable.
// Returns frequence in HZ.
// TODO: not yet implemented;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// uint32_t SysTick_get_ticks_per_second(void);


