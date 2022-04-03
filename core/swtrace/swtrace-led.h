
/*
================================================================================================#=
SW Trace LED

Provides some handy services for blinking LEDs.
These can be placed at will in various locations in the code
when other means for execution trace are not yet available.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#pragma once

// =============================================================================================#=
// Public API Services
// =============================================================================================#=


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Three functions: on, off, and toggle
// for each of the four external LEDs: red, green, blue, and yellow.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void swtrace_red_on(void);
void swtrace_red_off(void);
void swtrace_red_toggle(void);

void swtrace_green_on(void);
void swtrace_green_off(void);
void swtrace_green_toggle(void);

void swtrace_blue_on(void);
void swtrace_blue_off(void);

#define swtrace_blue_toggle() do {\
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_2); \
} while(0)


void swtrace_yellow_on(void);
void swtrace_yellow_off(void);
void swtrace_yellow_toggle(void);



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Do the needful for one-time startup initialization;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SW_Trace_LED_Init(void);


#if 0
// =============================================================================================#=
// =============================================================================================#=
// =============================================================================================#=
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
uint32_t SystemCoreClock_get_hclk_freq(void);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Use the given value to record the number of ticks-per-second
// that the Cortex-M System Timer (SysTick) has been configured for.
// TODO: not yet implemented;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void SysTick_record_ticks_per_second(uint32_t ticks_per_second);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Get the SysTick HCLK frequency as recorded in the global SystemCoreClock variable.
// Returns frequence in HZ.
// TODO: not yet implemented;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
uint32 SysTick_get_ticks_per_second(void);

#endif
