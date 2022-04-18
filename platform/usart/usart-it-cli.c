
// =============================================================================================#=
// USART INTERRUPT DRIVEN CLI API
// platform/usart/usart-it-cli.c
//
// Regarding interrupts...
// Bear in mind that there is one interrupt from the USART peripheral
// which can be enabled and disabled in the NVIC, USART2_IRQn for example.
// In addition, there are a number of status bits in the USART ISR register
// that will generate the peripheral level interrupt if enabled to do so.
// For example: the TXE bit in USART_ISR and TXEIE in USART_CR1.
// See Figure 443 on page 1383 of the reference manual.
//
// =============================================================================================#=

#include "platform/usart/usart-it-cli.h"

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

// -----------------------------------------------------------------------------+-
// Callback for 'input data available' as
// registered by the client application layer.
// -----------------------------------------------------------------------------+-
USART_IT_CLI_Input_Available_Callback input_data_avail = NULL;

// -----------------------------------------------------------------------------+-
// Internal Ring Buffers
// Size must be a power of two;
// -----------------------------------------------------------------------------+-
static uint8_t client_tx_queue_buff[256];   // TX chars from the client layer;
static uint8_t echo_queue_buff[32];         // RX chars from the user CLI; 

static ring_buffer client_tx_queue = {
    .buff = client_tx_queue_buff,
    .size = sizeof(client_tx_queue_buff),
    .tail = 0,
    .head = 0,
};

static ring_buffer echo_queue = {
    .buff = echo_queue_buff,
    .size = sizeof(echo_queue_buff),
    .tail = 0,
    .head = 0,
};


// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Helper function to do the needful when the USART TDR is empty;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void usart_tdr_empty(void)
{
    // @@@ if(rb_is_empty(&rb_usart_tx)) {
        // Disable the TXE interrupt as there is no more TX data to send;
        // It appears there is no way to explicitly clear TXE active bit?
        // @@@ LL_USART_DisableIT_TXE(USART2);
    // @@@ }
    // @@@ else {
        // Write the next outgoing byte to the TDR register;
        // this will clear the TXE bit;
        // @@@ uint8_t next_byte = rb_read_byte_from_head(&rb_usart_tx);
        // @@@ LL_USART_TransmitData8(USART2, next_byte);
    // @@@ }
    return;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART RDR Not Empty;
// Helper function to do the needful for the ISR;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void usart_rdr_notempty(void)
{
    // Read the RX byte; this also clears the RXNE bit;
    // @@@ uint8_t rx_byte = LL_USART_ReceiveData8(USART2);

    // @@@ if(rb_is_full(&rb_usart_rx)) {
        // All we can do is throw the byte away;
        // Perhaps someday we can increment an error counter here; TODO
    // @@@ }
    // @@@ else {
        // @@@ rb_write_byte_to_tail( &rb_usart_rx, rx_byte );

        // @@@ if(data_avail == NULL) return;   // no callback;

        // how many bytes are in the buffer waiting to be consumed by the application;
        // @@@ uint32_t bytes_avail = rb_bytes_available(&rb_usart_rx);

        // @@@ Trace_Blue_Toggle();

        // @@@ if(Rx_Detect_EOL && rx_byte == '\r') {
            // @@@ data_avail(bytes_avail, true);
        // @@@ }
        // @@@ else if( Rx_Buffer_Threshold > 0U && bytes_avail > Rx_Buffer_Threshold) {
            // @@@ data_avail(bytes_avail, false);
        // @@@ }
    // @@@ }
    return;
}

// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// TX API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Register the 'data available' callback;
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Set_Rx_Callback(USART_IT_CLI_Input_Available_Callback given_func_ptr)
{
    input_data_avail = given_func_ptr;
    return;
}


// =============================================================================================#=
// General Module APIs
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// USART PERIPHERAL INTERRUPT
//
// This should be invoked from the appropriate USART*_IRQHandler.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_CLI_ISR(void)
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
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// MODULE INIT
//
// One-time startup initialization for this software module;
//
// USART is connected to the Virtual Com Port
//     USART2
//     TX  PA2
//     RX  PA3
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
void USART_IT_CLI_Module_Init(uint32_t given_PCLK1_frequency_in_hertz)
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
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Helper function to signal to the USART module
// that new data is available in its Tx ring buffer;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static void tx_data_available(void)
{
    // Re-Enable the TXE interrupt
    // Our understanding is that if the TDR is empty (TXE=1) at the time we enable this interrupt,
    // then the USART peripheral will invoke the USART interrupt handler;
    //
    // If this interrupt happens to be already enabled,
    // then either the USART interrupt handler is already executing or
    // it will execute when the TDR becomes empty again.
    LL_USART_EnableIT_TXE(USART2);
};



// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// TX API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX APIs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

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





@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#endif

