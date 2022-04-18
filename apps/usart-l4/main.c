
/*
================================================================================================#=
MAIN
apps/usart-l4/main.c

This example describes how to send bytes over USART IP using
the STM32L4xx USART LL API.
Peripheral initialization done using LL unitary services functions.

CREDITS:
    STM32CubeL4/Projects/NUCLEO-L476RG/Examples_LL/USART/USART_Communication_Tx_IT/Src/main.c

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdint.h>
#include <string.h>

// Project dependencies
#include "core/swtrace/swtrace-led.h"
#include "mcu/clock/mco.h"
#include "mcu/clock/clock-tree-default-config.h"
#include "platform/usart/usart-it-cli.h"

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
static volatile int32_t  Blinky_Delay = 0x0000FFFF;

// -------------------------------------------------------------+-
// Flag to toggle when button is pressed;
// Unused currently;
// -------------------------------------------------------------+-
static volatile uint8_t toggle_button_press = 0;

// -------------------------------------------------------------+-
// Index of the next char to be sent;
// A zero value means we are ready to send the first char;
// -------------------------------------------------------------+-
static volatile uint8_t next_char_idx = 0;


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

uint8_t  Input_Buffer[32];
uint32_t Input_Buffer_Len = sizeof(Input_Buffer);

static void user_button_callback(void)
{
    Blinky_Delay -= 0xFF;
    if( Blinky_Delay < 0)
    {
        Blinky_Delay = 0x0000FFFF;
    }

    // @@@ uint32_t byte_count = USART_IT_BUFF_Rx_Get_Line(
        // @@@ Input_Buffer, Input_Buffer_Len
    // @@@ );
    // @@@ if( byte_count > 0) {
        // @@@ USART_IT_BUFF_Tx_Write_Best_Effort(Input_Buffer, byte_count);
    // @@@ }

    // Toggle button press flag; not really used;
    toggle_button_press ^= 1;
}

// -----------------------------------------------------------------------------+-
// Rx Data Available Callback;
// -----------------------------------------------------------------------------+-
// @@@ static void rx_data_avail_callback(uint32_t len, bool eol)
// @@@ {
    // @@@ Trace_Red_Toggle();

    // @@@ uint32_t byte_count = USART_IT_BUFF_Rx_Get_Line(
        // @@@ Input_Buffer, Input_Buffer_Len
    // @@@ );
    // @@@ if( byte_count > 0) {
        // @@@ USART_IT_BUFF_Tx_Write_Best_Effort(Input_Buffer, byte_count);
        // @@@ if(eol) {
            // @@@ // Add a Newline(LF) to the CR, replacing the NULL;
            // @@@ USART_IT_BUFF_Tx_Write_Best_Effort((uint8_t *)"\n", 1);
        // @@@ }
    // @@@ }
// @@@ }


// =============================================================================================#=
// Interrupt Request Handlers
// =============================================================================================#=
void EXTI15_10_IRQHandler(void) {

    // Check for the EXTI 13 interrupt flag
    if ((EXTI->PR1 & EXTI_PR1_PIF13) == EXTI_PR1_PIF13) {

        // Clear interrupt pending request by writing 1
        EXTI->PR1 |= EXTI_PR1_PIF13;

        user_button_callback();
    }
}


#if 0
// -----------------------------------------------------------------------------+-
// This is invoked when the USART is ready to receive
// the next outgoing TX byte.
// -----------------------------------------------------------------------------+-
static void TX_Next_Byte(void)
{
    LL_GPIO_SetOutputPin(   GPIOC, LL_GPIO_PIN_2);  // Blue LED
    if(next_char_idx == 0)
    {
        // Do nothing;
        // We want the button press to trigger TX of the first byte;
    }
    else if(next_char_idx < strlen(TX_Message_String))
    {
        uint8_t tx_byte = TX_Message_String[next_char_idx];

        USART_TX_This_Byte(USART_PERIPH_2, tx_byte);

        next_char_idx++;
    }
    else
    {
        // The whole string has been printed;
        next_char_idx = 0;

        // Things only work (able to print the string more than once ) when
        // We disable the interrupt here???????
        LL_USART_DisableIT_TXE(USART2);
    }
};
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART2 Interrupt Request
// (An external interrupt from the Cortex-M4 vector table;)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART2_IRQHandler(void)
{
    USART_IT_CLI_ISR();
};




#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    // User can add his own implementation to report the file name and line number,
    // ex: printf("Wrong parameters value: file %s on line %d", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
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

    // USART_IT_BUFF_Rx_Set_Callback(rx_data_avail_callback);
    // USART_IT_BUFF_Rx_Set_EOL_Detect(true);
    // USART_IT_BUFF_Rx_Set_Threshold_Detect(10U); // ten percent
    USART_IT_CLI_Module_Init( MCU_Clock_Get_PCLK1_Frequency_Hz() );

    while(1)
    {
        // Trace_Yellow_On();
        // for( uint32_t i=0; i<0xFFFFF; i++) {};
        // Trace_Yellow_Off();
        // for( uint32_t i=0; i<0xFFFFF; i++) {};
    }
}


