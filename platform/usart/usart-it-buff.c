
// =============================================================================================#=
// USART IT BUFF IMPLEMENTATION
// platform/usart/usart-it-buff.c
//
// This module has exclusive ownership of and responsibility for the USART peripherals on the MCU.
// It provides TX and RX services to the application and other platform components.
//
// Regarding interrupts...
// Bear in mind that there is one interrupt from the USART peripheral
// which can be enabled and disabled in the NVIC, USART2_IRQn for example.
// In addition, there are a number of status bits in the USART ISR register
// that will generate the peripheral level interrupt if enabled to do so.
// For example: the TXE bit in USART_ISR and TXEIE in USART_CR1.
// See Figure 443 on page 1383 of the reference manual.
//
// @@@ TODO:
// Support multiple USART Peripherals via USART_Periph_Num params
//
// =============================================================================================#=

#include "platform/usart/usart-it-buff.h"

#include <stdint.h>
#include <stdbool.h>

// Project dependencies
#include "core/swtrace/swtrace-led.h"

// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_bus.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_rcc.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_gpio.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_usart.h"


// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Structure for the ring buffer and its current state;
// @@@ TODO Refactor this into a separate stateless utility module.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
typedef struct
{
    uint8_t  *buff;   // Pointer to the first slot in the ring buffer.
    uint32_t  size;   // Number of slots in the buffer MUST be a power of two.
    uint32_t  tail;   // Identifies the slot to which we ADD a byte.
    uint32_t  head;   // Identifies the slot from which we REMOVE the next byte.

} ring_buffer;

// ---------------------------------------------------------------------+-
// Memory allocation for the usart TX and RX ring buffers;
// Size MUST BE a POWER of TWO;
// ---------------------------------------------------------------------+-
static uint8_t usart_tx_buffer[256];
static uint8_t usart_rx_buffer[256];

static ring_buffer rb_usart_tx = {
    .buff = usart_tx_buffer,
    .size = sizeof(usart_tx_buffer),
    .tail = 0,
    .head = 0,
};

static ring_buffer rb_usart_rx = {
    .buff = usart_rx_buffer,
    .size = sizeof(usart_rx_buffer),
    .tail = 0,
    .head = 0,
};


// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Ring Buffer Accessor Helper Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// These are simple low-level accessor functions.
// All these functions assume the caller has performed
// the necessary precondition checks and critical region protections.
//
// NOTICE! The approach used here assumes the buffer size is
// a power of two; this code won't work otherwise.
// Credit: https://www.snellman.net/blog/archive/2016-12-13-ring-buffers
//
// @@@ TODO Refactor this into a separate stateless utility module.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static uint32_t rb_bytes_available( ring_buffer *rb )
{
    return rb->tail - rb->head;
};

static uint32_t rb_slots_available( ring_buffer *rb )
{
    return rb->size - rb_bytes_available(rb);
};

static bool rb_is_empty( ring_buffer *rb )
{
    return rb->head == rb->tail;
};

static bool rb_is_full( ring_buffer *rb )
{
    return rb_bytes_available(rb) == rb->size;
};

static uint32_t rb_mask( ring_buffer *rb, uint32_t headortail )
{
    return (headortail & (rb->size - 1));
};

static void rb_write_byte_to_tail( ring_buffer *rb, uint8_t given_byte )
{
    rb->buff[rb_mask(rb, rb->tail++)] = given_byte;
    return;
};

static uint8_t rb_read_byte_from_head( ring_buffer *rb )
{
    return rb->buff[rb_mask(rb, rb->head++)];
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Helper function to signal that new data is available in the ring buffer;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void tx_data_available(void)
{
    // re-enable the TXE interrupt
    // Our working theory is that if the TDR is empty (TXE=1) at the time we enable this interrupt,
    // then the USART peripheral will generate the USART interrupt handler;
    // We'll see if it works like we hope it does.
    //
    // If this interrupt is already enabled,
    // then the USART interrupt handler is already running or
    // it will run when the TDR becomes empty again.
    LL_USART_EnableIT_TXE(USART2);
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Helper function to do the needful when the USART TDR is empty;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void usart_tdr_empty(void)
{
    if(rb_is_empty(&rb_usart_tx)) {
        // Disable the TXE interrupt as there is no more TX data to send;
        // It appears there is no way to explicitly clear TXE active bit?
        LL_USART_DisableIT_TXE(USART2);
    }
    else {
        // Write the next outgoing byte to the TDR register;
        // this will clear the TXE bit;
        uint8_t next_byte = rb_read_byte_from_head(&rb_usart_tx);
        LL_USART_TransmitData8(USART2, next_byte);
    }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART RDR Not Empty;
// Helper function to do the needful for the ISR;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void usart_rdr_notempty(void)
{
    // Read the RX byte; this also clears the RXNE bit;
    uint8_t rx_byte = LL_USART_ReceiveData8(USART2);

    if(rb_is_full(&rb_usart_rx)) {
        // All we can do is throw the byte away;
        // Perhaps someday we can increment an error counter here;
    }
    else {
        rb_write_byte_to_tail( &rb_usart_rx, rx_byte );
        Trace_Blue_Toggle();

        // @@@ TODO @@@
        //    invoke callback if either: rx_byte is a newline
        //    or: if number of bytes in rb exceeds some threshold
    }
}


// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Write the content of the given buffer into
// the USART TX ring buffer provided by this module;
// When there is insufficent space available in the ring buffer,
// the given content is silently thrown away;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_BUFF_Tx_Write_Best_Effort(uint8_t *buff_addr, uint8_t buff_len)
{
    uint32_t num_slots = rb_slots_available( &rb_usart_tx );

    if (num_slots < buff_len) {
        // insufficient space for given buffer;
        // buffer content is lost;
        return;
    }

    // Copy given content into buffer;
    for(int idx=0; idx<buff_len; idx++) {
        rb_write_byte_to_tail( &rb_usart_tx, buff_addr[idx] );
    }

    tx_data_available();
    return;
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Get Line
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_BUFF_Rx_Get_Line(
    uint8_t  *input_buffer,
    uint32_t  input_buffer_len)
{
    // allow room for the nul terminating char;
    int32_t  available_len = input_buffer_len-1;
    uint32_t idx=0;

    if(available_len <= 0) {
        // No room in the given buffer;
        input_buffer[0] = '\0';
    }
    else if(rb_is_empty(&rb_usart_rx)) {
        // No input available;
        input_buffer[0] = '\0';
    }
    else {
        while(idx<available_len) {
            uint8_t next_byte = rb_read_byte_from_head(&rb_usart_rx);
            input_buffer[idx++] = next_byte;
            if(next_byte == '\n') break;
            if(rb_is_empty(&rb_usart_rx)) break;
        }
        input_buffer[idx] = '\0';
    }
    // Trace_Red_Toggle();
    // Trace_Green_Toggle();
    // Trace_Yellow_Toggle();
    return idx;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART Peripheral Interrupt
// This should be invoked from the USART*_IRQHandler functions as appropriate.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_ISR(USART_Periph_Num given_usart)
{
    // TXE Event Flag => Transmit Data Register Empty;
    // Hardware sets this flag when data has been transferred
    // from the TDR to the TX shift register;
    //
    // If USART_CR1:TXEIE is enabled and USART_ISR:TXE is active
    // Then invoke the TXE callback.
    if(LL_USART_IsEnabledIT_TXE(USART2) && LL_USART_IsActiveFlag_TXE(USART2))
    {
        // Note: we assume this helper will be either writing to the TDR
        // which will clear the TXE active bit or
        usart_tdr_empty();
    }

    // RXNE Event Flag => Receive Data Register NotEmpty;
    // Hardware sets this flag when data has been transferred
    // from the RX shift register to the RDR;
    //
    if(LL_USART_IsEnabledIT_RXNE(USART2) && LL_USART_IsActiveFlag_RXNE(USART2))
    {
        // Note: we assume this helper will be reading from the RDR
        // which will clear the RXNE active bit;
        usart_rdr_notempty();
    }


    // @@@ FUTURE AS NEEDED @@@
    // @@@ if(LL_USART_IsEnabledIT_TC(USARTx_INSTANCE) && LL_USART_IsActiveFlag_TC(USARTx_INSTANCE))
    // @@@ {
        // @@@ LL_USART_ClearFlag_TC(USARTx_INSTANCE);

        /* Call function in charge of handling end of transmission of sent character and prepare next character transmission */
        // @@@ USART_CharTransmitComplete_Callback();
    // @@@ }

    // @@@ if(LL_USART_IsEnabledIT_ERROR(USARTx_INSTANCE) && LL_USART_IsActiveFlag_NE(USARTx_INSTANCE))
    // @@@ {
        /* Call Error function */
        // @@@ USART_Error_Callback();
    // @@@ }
};


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// One-time startup initialization of this module;
// @@@@ REFACTOR @@@@
//
    // Configure USARTx
    //
    // as per main.h...
    // USART is connected to the Virtual Com Port
    //     USART2
    //     TX  PA2
    //     RX  PA3
    //
    // According to section 6.9 on page 25 of UM1724,
    // the USART2 interface available on PA2 and PA3 of the STM32 microcontroller
    // can be connected to ST-LINK MCU, ST morpho connector, or to ARDUINO® connector.
    // The choice can be changed by setting the related solder bridges.
    // By default, the USART2 communication between the target STM32 and ST-LINK MCU is enabled.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_BUFF_Module_Init(uint32_t given_PCLK1_frequency_in_hertz)
{
    // Configure GPIO Pins
    //
    // Based on Table 16 on page 73 of the STM32L476xx Data Sheet,
    // we know that USART2 Tx and Rx are PA2 and PA3 respectively;
    // Based on Table 17 on page 92 of the STM32L476xx Data Sheet,
    // we know that PA2 and PA3 use Alternate Function 7 for USART;
    //
    // @@@ TODO: use Core Board Model
    //
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    // Configure Tx Pin
    LL_GPIO_SetPinMode(       GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(     GPIOA, LL_GPIO_PIN_2, LL_GPIO_AF_7);
    LL_GPIO_SetPinSpeed(      GPIOA, LL_GPIO_PIN_2, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType( GPIOA, LL_GPIO_PIN_2, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(       GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);

    // Configure Rx Pin
    LL_GPIO_SetPinMode(       GPIOA, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(     GPIOA, LL_GPIO_PIN_3, LL_GPIO_AF_7);
    LL_GPIO_SetPinSpeed(      GPIOA, LL_GPIO_PIN_3, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType( GPIOA, LL_GPIO_PIN_3, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(       GPIOA, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);

    // At the NVIC level, configure interrupt: USART2_IRQn
    NVIC_SetPriority( USART2_IRQn, 0 );
    NVIC_EnableIRQ(   USART2_IRQn    );

    // Select PCLK1 as the clock source for the USART2 peripheral;
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

    // Enable the peripheral bus clock to the USART2 peripheral;
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    // Disable USART prior to modifying its configuration registers (default);
    LL_USART_Disable(USART2);

    // Enable both TX and RX;
    // CR1: Transmit Enable and Receive Enable bits;
    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);

    // Configure character frame format;
    // 8 data bit, 1 start bit, 1 stop bit, no parity;
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

    // Configure No Hardware Flow control (default);
    LL_USART_SetHWFlowCtrl(USART2, LL_USART_HWCONTROL_NONE);

    // Set oversampling mode to 16-bit (default);
    LL_USART_SetOverSampling(USART2, LL_USART_OVERSAMPLING_16);

    // Set Baudrate to 115200
    LL_USART_SetBaudRate(
        USART2,
        given_PCLK1_frequency_in_hertz,
        LL_USART_OVERSAMPLING_16,
        115200
    );

    // Enable USART peripheral and wait for confirmation by polling
    // the Transmit Enable Acknowledge Flag
    // and the Receive Enable Acknowledge Flag
    LL_USART_Enable(USART2);
    while((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2)))) {};

    // Enable the needful interrupts
    LL_USART_EnableIT_RXNE(USART2);

    // LL_USART_EnableIT_ERROR(USART2);
    // LL_USART_ClearFlag_TXE(USART2);
    // LL_USART_EnableIT_TXE(USART2);
    // LL_USART_EnableIT_TC(USART2);
    // LL_USART_DisableIT_TXE(USART2);
};

#if 0
/**
  * @brief  Function called at completion of last byte transmission
  * @param  None
  * @retval None
  */
void USART_CharTransmitComplete_Callback(void)
{
    if(next_char_idx >= strlen(TX_Message_String))
    {
        next_char_idx = 0;

        LL_USART_DisableIT_TC(USARTx_INSTANCE);

        // LED_On();
    }
}

//
// Function called in case of error detected in USART IT Handler
//
void USART_Error_Callback(void)
{
    volatile uint32_t isr_reg;

    NVIC_DisableIRQ(USARTx_IRQn);

    /* Error handling example :
        - Read USART ISR register to identify flag that leads to IT raising
        - Perform corresponding error handling treatment according to flag
    */
    isr_reg = LL_USART_ReadReg(USARTx_INSTANCE, ISR);
    if (isr_reg & LL_USART_ISR_NE)
    {
        /* case Noise Error flag is raised : ... */
        LED_Blinking(LED_BLINK_FAST);
    }
    else
    {
        /* Unexpected IT source : Set LED to Blinking mode to indicate error occurs */
        LED_Blinking(LED_BLINK_ERROR);
    }
};
#endif

