
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
// an ISR is invoked to get the next byte from the ring buffer and write it to the TDR.
//
// RX Description
// future
// =============================================================================================#=

#pragma once

#include <stdint.h>
#include <stdbool.h>


// -----------------------------------------------------------------------------+-
// This module (will someday) support multiple USART peripherals;
// -----------------------------------------------------------------------------+-
typedef enum
{
    USART_PERIPH_1 = 0,
    USART_PERIPH_2 = 1,
    USART_PERIPH_3 = 2,
} USART_Periph_Num;


// -----------------------------------------------------------------------------+-
// One-time startup initialization for this module;
// -----------------------------------------------------------------------------+-
void USART_IT_BUFF_Module_Init(
    uint32_t  given_PCLK1_frequency_in_hertz
);

// -----------------------------------------------------------------------------+-
// USART Peripheral Interrupt
// This should be invoked from the USART*_IRQHandler functions as appropriate.
// -----------------------------------------------------------------------------+-
void USART_ISR(USART_Periph_Num given_usart);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// TX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Write the content of the given buffer into
// the module's USART TX ring buffer.
// Warning: when there is insufficent space available in the ring buffer,
// the given content is silently thrown away;
// If you cannot allow your content to be lost, and you can afford to wait,
// use the following API call to first check for available slots.
// -----------------------------------------------------------------------------+-
void USART_IT_BUFF_Tx_Write_Best_Effort(uint8_t *buff_addr, uint8_t buff_len);

// -----------------------------------------------------------------------------+-
// Returns the number of slots available for new outgoing TX bytes.
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_BUFF_Tx_Slots_Available(void);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Function pointer type for the data available callback;
// -----------------------------------------------------------------------------+-
typedef void (*USART_IT_BUFF_Rx_Data_Available_Callback)(uint32_t len, bool eol);


// -----------------------------------------------------------------------------+-
// Register the 'data available' callback;
// The given function will be called when there is
// Rx data available for consumption;
// -----------------------------------------------------------------------------+-
void USART_IT_BUFF_Rx_Set_Callback(USART_IT_BUFF_Rx_Data_Available_Callback func_ptr);


// -----------------------------------------------------------------------------+-
// This USART module can be configured to invoke
// the callback when it receives a Carriage Return character;
// -----------------------------------------------------------------------------+-
void USART_IT_BUFF_Rx_Set_EOL_Detect(bool on_or_off);


// -----------------------------------------------------------------------------+-
// This USART module can be configured to invoke
// the callback when the number of buffered characters
// exceeds the given threshold relative to the full size
// of the module's internal ring buffer.
// -----------------------------------------------------------------------------+-
void USART_IT_BUFF_Rx_Set_Threshold_Detect(uint8_t percentage_full);


// -----------------------------------------------------------------------------+-
// Get Line
//
// Reads bytes from the given USART into the given input buffer;
// Reading will stop when either:
//     input_buffer_len - 1 bytes are read,
//     a newline character is read,
//     there are no more input bytes available,
//
// Returns the number of bytes read into the input buffer excluding
// the terminating NUL character, i.e., the length of the resultant input string;
//
// Upon return, input_buffer will always point to a NUL terminated string;
// That terminated string may be empty, i.e., zero length;
//
// TODO: Future: add USART id as third param;
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_BUFF_Rx_Get_Line(
    uint8_t  *input_buffer,
    uint32_t  input_buffer_len
);


