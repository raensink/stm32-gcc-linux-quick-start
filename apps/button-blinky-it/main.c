
/*
================================================================================================#=
MAIN
apps/button-blinky-it/main.c

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdint.h>

#include "CMSIS/Device/ST/STM32F0xx/Include/stm32f091xc.h"

#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_bus.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_system.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_rcc.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_gpio.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_exti.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_utils.h"


// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// -----------------------------------------------------+-
// Frequency of the HCLK in Hz;
// Set by the clock configuration code;
// -----------------------------------------------------+-
static uint32_t HCLK_Frequency_Hz;

// -----------------------------------------------------+-
// Frequency of the APB1 PCLK1 in Hz;
// Set by the clock configuration code;
// -----------------------------------------------------+-
// static uint32_t PCLK1_Frequency_Hz;

// -----------------------------------------------------+-
// Frequency of the SysTick Timer in ticks-per-second;
// Set by the clock configuration code;
// -----------------------------------------------------+-
static uint32_t TicksPerSecond;

// -----------------------------------------------------+-
// -----------------------------------------------------+-
static volatile int32_t  Blinky_Delay = 0x000FFFFF;


// =============================================================================================#=
// Private Internal Helper Functions
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// System Clock Configuration
// -----------------------------------------------------------------------------+-
static void system_clock_config(void)
{
    // @@@ Someday move this to mcu/core/FLASH @@@
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    // Configure the PLL to (HSI / 2) * 12 = 48MHz.
    // Use a PLLMUL of 0xA for *12, and keep PLLSRC at 0
    // to use (HSI / PREDIV) as the core source. HSI = 8MHz.
    RCC->CFGR  &= ~(RCC_CFGR_PLLMUL |
                    RCC_CFGR_PLLSRC);
    RCC->CFGR  |=  (RCC_CFGR_PLLSRC_HSI_DIV2 |
                    RCC_CFGR_PLLMUL12);
    // Turn the PLL on and wait for it to be ready.
    RCC->CR    |=  (RCC_CR_PLLON);
    while (!(RCC->CR & RCC_CR_PLLRDY)) {};
    // Select the PLL as the system clock source.
    RCC->CFGR  &= ~(RCC_CFGR_SW);
    RCC->CFGR  |=  (RCC_CFGR_SW_PLL);
    while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) {};

    // Set the global clock frequency variable.
    HCLK_Frequency_Hz = 48000000;

    // Configure the Cortex-M SysTick source for 1000 ticks per second given the HCLK frequency;
    // Sets RELOAD register value to (HCLKFrequency / TicksPerSecond) - 1;
    // Clears the counter value;
    // Sets CSR:Clock source to use processor clock rather than external clock;
    // Leaves CSR:TickInt unchanged (zero?) so assume no SysTick exception? But then why set it?
    // Sets CSR:Enable to enable the counter
    // TicksPerSecond = 1000U;
    LL_InitTick(HCLK_Frequency_Hz, TicksPerSecond);

    // Set the CMSIS SystemCoreClock variable to record the freq of the HCLK;
    LL_SetSystemCoreClock(HCLK_Frequency_Hz);
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Configure Microcoontroller Clock Output (MCO)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void mco_config(void)
{
    // Configure GPIO PIN
    //
    // Based on Table 16 on page 84 of the STM32L476xx Data Sheet, we know that the MCO is on PIN PA8.
    // Enable the peripheral bus clock to the GPIOA peripheral which is on the AHB2 bus
    // and configure the pin accordingly for MCO.
    LL_AHB1_GRP1_EnableClock( LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinMode(       GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(    GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_0);
    LL_GPIO_SetPinOutputType( GPIOA, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(       GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinSpeed(      GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);

    // Configure the MCO source selection and divider.
    // Note: use the MCO prescaler divider as needed to ensure that
    // the output signal can by your Saleae or other instrument.
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_SYSCLK, LL_RCC_MCO1_DIV_8);

    // Some handy MCO alternative configurations:
    // LL_RCC_ConfigMCO( LL_RCC_MCO1SOURCE_MSI, LL_RCC_MCO1_DIV_1);
    // LL_RCC_ConfigMCO( LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_1);
    // LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_SYSCLK, LL_RCC_MCO1_DIV_8);
};


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
static void user_button_init(void)
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
// =============================================================================================#=
int main(void)
{
    // Initialize Clock Tree and SysTick at startup;
    system_clock_config();

    // Configure Microcontroller Clock Output (MCO)
    // The MCO is on PA8 which,
    // according to Figure 24 on page 35 of UM1724 STM32 Nucleo-64 Boards User Manual,
    // is available on CN10 Pin #23
    mco_config();

    // Initialize GPIO peripherals for LEDs;
    init_led_gpio();

    user_button_init();

    // Forever toggle some LEDs...
    while (1)
    {
        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5); // On-Board Green
        LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_2); // Blue LED

        // Busy wait to delay;
        for( uint32_t i=0; i<Blinky_Delay; i++) {}
    }
}


