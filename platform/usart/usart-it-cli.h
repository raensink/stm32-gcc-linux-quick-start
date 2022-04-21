
// =============================================================================================#=
// USART INTERRUPT DRIVEN CLI API
// platform/usart/usart-it-cli.h
//
// This module has exclusive ownership of and responsibility for
// a single USART dedicated to the command line interface.
// It provides TX and RX services to a CLI client to support:
//     Command Line Input
//     Command Response Output
//     SW Trace Output
//
// SPDX-License-Identifier: MIT-0
// =============================================================================================#=

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_usart.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// General Module APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// One-time startup initialization for this module;
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Module_Init(
    uint32_t  given_PCLK1_frequency_in_hertz
);

// -----------------------------------------------------------------------------+-
// USART Peripheral Interrupt
// This should be invoked from the USART*_IRQHandler function.
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_ISR(void);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// TX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// USART CLI PUT RESPONSE
// USART CLI PUT TRACE
//
// Write the given content into
// the Command Response ring buff or the Trace Output ring buff, respectively.
//
// Warning: when there is insufficent space available in either ring buffer,
// the given content is silently thrown away;
// If the client cannot allow it's content to be lost, and if it can afford to wait,
// use the 'slots available' API call to first check for available TX slots.
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Put_Response(uint8_t *buff_addr, uint8_t buff_len);
void USART_IT_CLI_Put_Trace(uint8_t *buff_addr, uint8_t buff_len);

// -----------------------------------------------------------------------------+-
// Returns the number of slots available for new outgoing TX bytes.
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_CLI_Response_Slots_Available(void);
uint32_t USART_IT_CLI_Trace_Slots_Available(void);


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Callback Function Pointer Type
//
// Function pointer type for callback: command line input available;
// The length parameter indicates the string length of the command input;
// -----------------------------------------------------------------------------+-
typedef void (*USART_IT_CLI_Input_Available_Callback)(uint32_t len);

// -----------------------------------------------------------------------------+-
// Register Rx Callback
//
// Register the 'data available' callback;
// The given function will be called when there is
// command line input data available for consumption;
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Register_Rx_Callback(USART_IT_CLI_Input_Available_Callback func_ptr);

// -----------------------------------------------------------------------------+-
// Get Line
//
// Reads the next line of input from the command line;
// Reading will stop when either:
//     input_buffer_len - 1 bytes are read,
//     a newline character is read,
//     there are no more input bytes available,
//
// Upon return, input_buffer will always point to a NUL terminated string;
// That terminated string may, however, be empty.
//
// Returns the string length of the input string that has been read into the buffer;
// the terminating NUL character is excluded from string length;
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_CLI_Get_Line(
    uint8_t  *input_buffer,
    uint32_t  input_buffer_len
);

