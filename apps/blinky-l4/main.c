
/*
================================================================================================#=
MAIN
apps/button-blinky-it/main.c

This application will flash an LED at various rates.
Pressing a button will change the rate at which it flashes.
Four external LEDs can be connected for additional blinkiness
and to support Software Trace for debugging.

DEPENDENCIES:
* MCU & Core services provided by this quick-start project.
* STM32F0 HAL Low Level Drivers
* RM0091 STM32F0 Reference Manual
* STM32F0 MCU
* NUCLEO-F091RC target board
* Four external LEDs for software trace blinkiness.
* Analyzer/Scope to measure MCO clock output.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdint.h>

// Project dependencies
#include "core/swtrace/swtrace-led.h"
#include "mcu/clock/mco.h"
#include "mcu/clock/clock-tree-default-config.h"

// MCU Device Definition
#include "CMSIS/Device/ST/STM32L4xx/Include/stm32l476xx.h"

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_bus.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_exti.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_gpio.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_system.h"


// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// -----------------------------------------------------+-
// Defines how long to wait between blinkies;
// Volatile because this is changed by the button ISR.
// -----------------------------------------------------+-
static volatile int32_t  Blinky_Delay = 0x0007FFFF;


// =============================================================================================#=
// Private Internal Helper Functions
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// User Button Configuration
// -----------------------------------------------------------------------------+-
static void user_button_config(void)
{
    // -------------------------------------------------------------+-
    // Enable the clock to the GPIO and SYSCFG peripheral;
    // ToDo: abstract this and make it part of the mapping?
    // Move this to core?
    // -------------------------------------------------------------+-
#if defined(MCUFAM_STM32F0)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
#elif defined(MCUFAM_STM32L4)
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
#else
    #error "MCU Family Name Not Defined."
#endif

    // -------------------------------------------------------------+-
    // Configure the input pin;
    // -------------------------------------------------------------+-
    LL_GPIO_SetPinMode(
        ON_BOARD_USER_BUTTON_PERIPH,
        ON_BOARD_USER_BUTTON_PIN,
        LL_GPIO_MODE_INPUT
    );

    LL_GPIO_SetPinPull(
        ON_BOARD_USER_BUTTON_PERIPH,
        ON_BOARD_USER_BUTTON_PIN,
        LL_GPIO_PULL_NO
    );

    // -------------------------------------------------------------+-
    // We choose to use the EXTI13 external interrupt line
    // and we configure it's source input as the GPIOC peripheral.
    //
    // In the STM32F0 RM0091 Ref Manual...
    // See Figure 24 on page 214 and See Section 9.1.5 on page 171.
    //
    // In the STM32L4 RM0351 Ref Manual...
    // See Figure 34 on page 403 and See Section 9.2.6 on page 321.
    // -------------------------------------------------------------+-
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

    // -------------------------------------------------------------+-
    // Enable the EXTI13 external interrupt line
    // and configure it for a falling edge trigger.
    // -------------------------------------------------------------+-
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);

    // -------------------------------------------------------------+-
    // On the STM32F0 NVIC, EXTI lines 4 thru 15 are consolidated
    // into a single interrupt vector: EXTI4_15.
    // See Table 36 on page 210 of RM0091.
    //
    // On the STM32L4 NVIC, EXTI lines 10 thru 15 are consolidated
    // into a single interrupt vector: EXTI15_10.
    // See Table 58 on page 397 of RM0351.
    //
    // Enable this this IRQ and set it's priority.
    // -------------------------------------------------------------+-
#if defined(MCUFAM_STM32F0)
    NVIC_SetPriority(EXTI4_15_IRQn, 3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
#elif defined(MCUFAM_STM32L4)
    NVIC_SetPriority(EXTI15_10_IRQn, 3);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
#else
    #error "MCU Family Name Not Defined."
#endif
}


// -----------------------------------------------------------------------------+-
// On-Board User Button Application Callback
// -----------------------------------------------------------------------------+-
static void user_button_callback(void)
{
    Blinky_Delay = Blinky_Delay - 0x00000FFFF;
    Trace_Red_Toggle();
    if( Blinky_Delay < 0)
    {
        Trace_Green_Toggle();
        Blinky_Delay = 0x0007FFFF;
    }
}


// =============================================================================================#=
// Interrupt Request Handlers
// =============================================================================================#=
#if defined(MCUFAM_STM32F0)
void EXTI4_15_IRQHandler(void) {
    // Check for the EXTI 13 pending interrupt flag in the pending register;
    if ((EXTI->PR & EXTI_PR_PIF13) == EXTI_PR_PIF13) {
        // Clear pending interrupt flag by writing a 'one'.
        EXTI->PR |= EXTI_PR_PIF13;

        user_button_callback();
    }
    else {
        // We would not expect any other interrupts to be pending at this point
        // because we have not configured any other EXTIs in the 4-15 range.
    }
}

#elif defined(MCUFAM_STM32L4)
void EXTI15_10_IRQHandler(void) {
    // Check for the EXTI 13 pending interrupt flag in the pending register;
    if ((EXTI->PR1 & EXTI_PR1_PIF13) == EXTI_PR1_PIF13) {
        // Clear pending interrupt flag by writing a 'one'.
        EXTI->PR1 |= EXTI_PR1_PIF13;

        user_button_callback();
    }
    else {
        // We would not expect any other interrupts to be pending at this point
        // because we have not configured any other EXTIs in the 10-15 range.
    }
}

#else
    #error "MCU Family Name Not Defined."
#endif


// =============================================================================================#=
// MAIN
// =============================================================================================#=
int main(void)
{
    // TODO: Explain what's going on here.
    // Refactor? Move?
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    // Initialize Clock Tree and SysTick at startup;
    MCU_Clock_Tree_Default_Config();

    // Configure Microcontroller Clock Output
    MCU_Clock_MCO_Config();

    // Application specific init.
    SW_Trace_External_LED_Init();
    SW_Trace_OnBoard_LED_Init();
    user_button_config();

    // Forever toggle some LEDs...
    while (1)
    {
        Trace_OnBrdGreen_On();
        Trace_Blue_Off();

        for( uint32_t i=0; i<0x000000FF; i++) {

            // Blinking blue and green;
            Trace_Blue_Toggle();
            Trace_OnBrdGreen_Toggle();

            // Busy wait to delay;
            // Duration can vary depending on delay value;
            for( uint32_t i=0; i<Blinky_Delay; i++) {}
        }

        Trace_Yellow_On();
        Trace_OnBrdGreen_On();
        for( uint32_t i=0; i<0x00F77777; i++) {}

        Trace_Yellow_Off();
        Trace_OnBrdGreen_Off();
        for( uint32_t i=0; i<0x00700000; i++) {}
    }
}


