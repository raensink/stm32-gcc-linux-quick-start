
// =============================================================================================#=
// USART IT BUFF API
// platform/usart/usart-it-buff.h
//
// This module has exclusive ownership of and responsibility for the USART peripherals on the MCU.
// It provides TX and RX services to the application and other platform components.
//
// TX Description
// The outgoing bytes on the transmit side are accumulated in a fixed-size ring buffer.
// Transmission is interrupt driven: when the USART TDR register is empty,
// an ISR is invokde to get the next byte from the ring buffer and write it to the TDR.
//
// RX Description
// future
// =============================================================================================#=

#pragma once

#include <stdint.h>


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// This module (will someday) support multiple USART peripherals;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
typedef enum
{
    USART_PERIPH_1 = 0,
    USART_PERIPH_2 = 1,
    USART_PERIPH_3 = 2,
} USART_Periph_Num;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// One-time startup initialization for this module;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_BUFF_Module_Init(
    uint32_t  given_PCLK1_frequency_in_hertz
);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Write the content of the given buffer into
// the USART TX ring buffer provided by this module;
// When there is insufficent space available in the ring buffer,
// the given content is silently thrown away;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_BUFF_Tx_Write_Best_Effort(uint8_t *buff_addr, uint8_t buff_len);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART Peripheral Interrupt
// This should be invoked from the USART*_IRQHandler functions as appropriate.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_ISR(USART_Periph_Num given_usart);


