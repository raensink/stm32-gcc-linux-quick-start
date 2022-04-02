
/*
================================================================================================#=
MAIN
apps/button-blinky-it/main.c

This application will flash an LED at various rates.
Pressing a button will change the rate at which it flashes.
Four external LEDs can be connected for additional blinkiness
and to support Software Trace Debugging.

DEPENDENCIES:
    MCU & Core services provided by this quick-start project.
    STM32F0 HAL Low Level Drivers
    STM32F0
    NUCLEO-F091RC target board

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdint.h>

#include "mcu/clock/mco.h"
#include "mcu/clock/clock-tree-default-config.h"


#include "CMSIS/Device/ST/STM32F0xx/Include/stm32f091xc.h"

// STM32 Low Level Drivers
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_bus.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_exti.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_gpio.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_system.h"


// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// -----------------------------------------------------+-
// Defines how long to wait between blinkies;
// Volatile because this is changed by the button ISR.
// -----------------------------------------------------+-
static volatile int32_t  Blinky_Delay = 0x000FFFFF;


// =============================================================================================#=
// Private Internal Helper Functions
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// -----------------------------------------------------------------------------+-
static void init_led_gpio(void)
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
};



// -----------------------------------------------------------------------------+-
// -----------------------------------------------------------------------------+-
static void user_button_config(void)
{
    // -----------------------------------------------------------------------------+-
    // GPIO PC13;
    // On-Board User Button (B1)
    //
    // We know from the UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
    // that the blud B1 user button is connected PC13;
    // See Figure 3 on page 13, and Section 6.5 on page 23;
    // -----------------------------------------------------------------------------+-

    // Enable the GPIOC peripheral and configure the pin;
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);

    // Connect PortC to External Line #13;
    // In the RM0091 Ref Manual...
    // See Section 9.1.5 on page 171;
    // and Figure 24 on page 241;
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    // Configure EXTI13 in the EXTI Peripheral;
    // See 14.3.1 EXTI Block Diagram in the RM0351 Ref Manual;
    // Enable a falling trigger External line 13 Interrupt
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);

    // Configure NVIC for USER_BUTTON_EXTI_IRQn
    NVIC_SetPriority(EXTI4_15_IRQn, 3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// -----------------------------------------------------------------------------+-
// On-Board User Button ISR
// -----------------------------------------------------------------------------+-
static void UserButton_Callback(void)
{
    Blinky_Delay = Blinky_Delay - 0x00000FFFF;
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_0); // Red External LED
    if( Blinky_Delay < 0)
    {
        LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_1);
        Blinky_Delay = 0x000FFFFF;
    }
}

void EXTI4_15_IRQHandler(void) {
    // Check for the EXTI 13 pending interrupt flag in the pending register;
    if ((EXTI->PR & EXTI_PR_PIF13) == EXTI_PR_PIF13) {
        // Clear pending interrupt flag by writing 1
        EXTI->PR |= EXTI_PR_PIF13;
        UserButton_Callback();
    }
}


// =============================================================================================#=
// MAIN
// =============================================================================================#=
int main(void)
{
    // Initialize Clock Tree and SysTick at startup;
    mcu_clock_tree_default_config();

    // TODO:
    // Explain what's going on here.
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    // Configure Microcontroller Clock Output
    mcu_clock_mco_config();

    // Application specific init.
    init_led_gpio();
    user_button_config();

    // Forever toggle some LEDs...
    while (1)
    {
        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5); // On-Board Green
        LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_2); // Blue LED

        // Busy wait to delay;
        // Duration can vary depending on 
        for( uint32_t i=0; i<Blinky_Delay; i++) {}
    }
}


