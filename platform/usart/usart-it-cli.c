
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
// SPDX-License-Identifier: MIT-0
// =============================================================================================#=

#include "platform/usart/usart-it-cli.h"

#include <string.h>

// Project dependencies
#include "core/swtrace/swtrace-led.h"
#include "platform/util/ring-buffer.h"

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

static Ring_Buffer client_tx_queue = {
    .buff = client_tx_queue_buff,
    .size = sizeof(client_tx_queue_buff),
    .tail = 0,
    .head = 0,
};

static Ring_Buffer echo_queue = {
    .buff = echo_queue_buff,
    .size = sizeof(echo_queue_buff),
    .tail = 0,
    .head = 0,
};


// -----------------------------------------------------------------------------+-
// Pending Command Buffer (PCB)
// -----------------------------------------------------------------------------+-
typedef struct
{
    uint8_t  *buff;      // pending command buffer
    uint32_t  size;      // buffer size
    uint32_t  tail_idx;  // the next input char goes here;
    uint32_t  read_idx;  // this is the next char to be read;
} PCB_Struct;

static uint8_t pcb_a[128];
static uint8_t pcb_b[128];

static PCB_Struct Double_PCB[] = {
    {
        .buff = pcb_a,
        .size = sizeof(pcb_a),
        .tail_idx = 0,
        .read_idx = 0,
    },
    {
        .buff = pcb_b,
        .size = sizeof(pcb_b),
        .tail_idx = 0,
        .read_idx = 0,
    },
};

#define BUFF_NONE     (-1)
#define BUFF_A         (0)
#define BUFF_B         (1)
static int8_t  Cmd_In_Progress = BUFF_A;
static int8_t  Cmd_Ready       = BUFF_NONE;


// -----------------------------------------------------------------------------+-
// The TXE ISR needs a state model to keep track of
// what it should be doing on each invocation.
// -----------------------------------------------------------------------------+-
typedef enum {
    eTxState_TXE_Disabled,
    eTxState_CLI_Refresh,
    eTxState_Echo_Queue,
    eTxState_Client_Queue,
} eTxState;

static eTxState  TXE_ISR_State = eTxState_TXE_Disabled;

// A couple "super states" that trump the TXE_ISR_State;
static bool  LF_NEEDED = false;
static bool  CR_NEEDED = false;

static char *cli_prompt = "cli> ";


// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// PCB Accessor Helper Functions
// -----------------------------------------------------------------------------+-
static void pcb_reset(PCB_Struct *pcb)
{
    pcb->read_idx = 0;
    strcpy((char *)pcb->buff, cli_prompt);
    pcb->tail_idx = strlen(cli_prompt);
}

static void pcb_reset_refresh(PCB_Struct *pcb)
{
    pcb->read_idx = 0;
}

static void pcb_reset_get_cmd(PCB_Struct *pcb)
{
    pcb->read_idx = strlen(cli_prompt);
}

static bool pcb_is_full(PCB_Struct *pcb)
{
    return(pcb->tail_idx >= pcb->size);
}

static uint32_t pcb_strlen(PCB_Struct *pcb)
{
    return(pcb->tail_idx - strlen(cli_prompt));
}

static bool pcb_is_empty(PCB_Struct *pcb)
{
    return(pcb->tail_idx == strlen(cli_prompt));
}

static void pcb_add_char(
    PCB_Struct *pcb,
    uint8_t     new_byte)
{
    pcb->buff[pcb->tail_idx] = new_byte;
    pcb->tail_idx++;
}

static bool pcb_read_next_char(
    PCB_Struct *pcb,
    uint8_t    *new_byte)
{
    if(pcb->read_idx < pcb->tail_idx) {
        *new_byte = pcb->buff[pcb->read_idx];
        pcb->read_idx++;
        return true;
    }
    else {
        return false;
    }
}

// -----------------------------------------------------------------------------+-
// Add a new incoming character to the pending command buffer;
// -----------------------------------------------------------------------------+-
static eTxState process_new_input_char(uint8_t new_byte)
{
    bool        eol = false;
    PCB_Struct *pcb_ptr = &Double_PCB[Cmd_In_Progress];

    // Stay in echo queue state unless we find a reason to do otherwise;
    eTxState  next_state = eTxState_Echo_Queue;

    // Test for 'Enter' keypress
    if(new_byte == '\r') {
        // around here, we prefer our EOL delimiters as newlines;
        pcb_add_char(pcb_ptr, '\n');
        eol = true;
    }
    else {
        pcb_add_char(pcb_ptr, new_byte);
    }
    // TODO: Future, process backspace

    // if the buffer is full, count that as an EOL
    if(pcb_is_full(pcb_ptr)) {
        eol = true;
    }

    // Process EOL;
    if(eol) {
        if(1 == pcb_strlen(pcb_ptr)) {
            // User just hit enter key at prompt
            // Reset the PCB and refresh the CLI;
            pcb_reset(pcb_ptr);
            next_state = eTxState_CLI_Refresh;
        }
        else {
            // Notify client that the input command line is ready for consumption;
            Cmd_Ready = Cmd_In_Progress;
            if(input_data_avail != NULL) {
                uint32_t len = pcb_strlen(pcb_ptr);
                // INVOKE CALLBACK!
                input_data_avail(len);
            }

            // Swap the double buffers and reset the one newly in-progress;
            Cmd_In_Progress = (Cmd_Ready == BUFF_A) ? BUFF_B : BUFF_A;
            pcb_reset(&Double_PCB[Cmd_In_Progress]);
        }
    }
    return next_state;
}

static eTxState service_the_echo_queue(uint8_t *next_byte)
{
    eTxState    next_state;

    // Assumes the echo queue is not empty;
    *next_byte = RB_Read_Byte_From_Head(&echo_queue);
    next_state = process_new_input_char(*next_byte);

    if(*next_byte == '\r') {
        // record the need to xmit an LF to accompany the CR;
        // this should happen on the next invocation of the ISR;
        LF_NEEDED = true;
    }
    return next_state;
}

static eTxState service_the_client_queue(uint8_t *next_byte)
{
    eTxState    next_state;

    // Assumes the client queue is not empty;
    *next_byte = RB_Read_Byte_From_Head(&client_tx_queue);
    next_state = eTxState_Client_Queue;

    if(*next_byte == '\n') {
        // record the need to xmit an CR to accompany the LF;
        // this will happen on the next invocation of the ISR;
        CR_NEEDED = true;
    }
    return next_state;
}

// -----------------------------------------------------------------------------+-
// State Handlers
// -----------------------------------------------------------------------------+-
static eTxState handle_txe_disabled_state(uint8_t *next_byte)
{
    eTxState    next_state;

    if(RB_Is_Not_Empty(&echo_queue)) {
        next_state = service_the_echo_queue(next_byte);
    }
    else if(RB_Is_Not_Empty(&client_tx_queue)) {
        next_state = service_the_client_queue(next_byte);
    }
    else {
        next_state = eTxState_TXE_Disabled;
    }
    return next_state;
}

static eTxState handle_cli_refresh_state(uint8_t *next_byte)
{
    eTxState    next_state;
    PCB_Struct *pcb_ptr = &Double_PCB[Cmd_In_Progress];

    if(pcb_read_next_char(pcb_ptr, next_byte)) {
        next_state = eTxState_CLI_Refresh;
    }
    else if(RB_Is_Not_Empty(&echo_queue)) {
        next_state = service_the_echo_queue(next_byte);
    }
    else if(RB_Is_Not_Empty(&client_tx_queue)) {
        next_state = service_the_client_queue(next_byte);
    }
    else {
        next_state = eTxState_TXE_Disabled;
    }
    return next_state;
}

static eTxState handle_echo_queue_state(uint8_t *next_byte)
{
    eTxState    next_state;

    if(RB_Is_Not_Empty(&echo_queue)) {
        next_state = service_the_echo_queue(next_byte);
    }
    else if(RB_Is_Not_Empty(&client_tx_queue)) {
        next_state = service_the_client_queue(next_byte);
    }
    else {
        next_state = eTxState_TXE_Disabled;
    }
    return next_state;
}

static eTxState handle_client_queue_state(uint8_t *next_byte)
{
    eTxState    next_state;
    PCB_Struct *pcb_ptr;

    if(RB_Is_Not_Empty(&client_tx_queue)) {
        // Continue servicing the client queue until it is empty;
        next_state = service_the_client_queue(next_byte);
    }
    else {
        // Then refresh the user's command line;
        pcb_ptr = &Double_PCB[Cmd_In_Progress];
        pcb_reset_refresh(pcb_ptr);
        pcb_read_next_char(pcb_ptr, next_byte);
        next_state = eTxState_CLI_Refresh;
    }
    return next_state;
}



// -----------------------------------------------------------------------------+-
// Helper function to signal to the USART peripheral
// that new TX data is available for transmission onto the wire;
// -----------------------------------------------------------------------------+-
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

// -----------------------------------------------------------------------------+-
// Helper function to do the needful when the USART TDR is empty;
// -----------------------------------------------------------------------------+-
static void usart_tdr_empty(void)
{
    eTxState next_state;
    uint8_t  next_byte;

    if(LF_NEEDED) {
        // Do not change the current state;
        // Transmit a Line Feed character;
        LL_USART_TransmitData8(USART2, '\n');
        LF_NEEDED = false;
        return;
    }
    if(CR_NEEDED) {
        // Do not change the current state;
        // Transmit a Carriage Return character;
        LL_USART_TransmitData8(USART2, '\r');
        CR_NEEDED = false;
        return;
    }

    switch (TXE_ISR_State) {

    case eTxState_TXE_Disabled:
        next_state = handle_txe_disabled_state(&next_byte);
        // Trace_Red_Toggle();
        break;

    case eTxState_CLI_Refresh:
        next_state = handle_cli_refresh_state(&next_byte);
        break;

    case eTxState_Echo_Queue:
        next_state = handle_echo_queue_state(&next_byte);
        break;

    case eTxState_Client_Queue:
        next_state = handle_client_queue_state(&next_byte);
        break;

    default:
        // ADD ASSERT false HERE;
        break;
    }

    if(next_state == eTxState_TXE_Disabled) {
        LL_USART_DisableIT_TXE(USART2);
    }
    else {
        // Write the next tx byte to the TDR register;
        // this will clear the TXE bit;
        LL_USART_TransmitData8(USART2, next_byte);
    }
    TXE_ISR_State = next_state;
    return;
}

// -----------------------------------------------------------------------------+-
// USART RDR Not Empty;
// Helper function to do the needful for the ISR;
//
// On the RX side of things, the ISR simply puts
// each received character into the echo queue and
// then let the TX ISR handle all of the processing.
// -----------------------------------------------------------------------------+-
static void usart_rdr_notempty(void)
{
    // Read the RX byte; this also clears the RXNE bit;
    uint8_t rx_byte = LL_USART_ReceiveData8(USART2);

    if(RB_Is_Full(&echo_queue)) {
        // All we can do is throw the byte away;
        // Perhaps someday we can increment an error counter here; TODO
    }
    else {
        //Trace_Blue_Toggle();
        RB_Write_Byte_To_Tail( &echo_queue, rx_byte );
        tx_data_available();
    }
    return;
}

// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// TX API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// PUT BEST EFFORT
//
// Write the given content into the Client Tx Queue (ring buffer);
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Put_Best_Effort(uint8_t *given_buff_addr, uint8_t given_buff_len)
{
    uint32_t num_slots = RB_Slots_Available(&client_tx_queue);

    if (num_slots < given_buff_len) {
        // insufficient space for given content;
        // buffer content is lost;
        return;
    }

    // Copy given content into buffer;
    for(int idx=0; idx<given_buff_len; idx++) {
        RB_Write_Byte_To_Tail( &client_tx_queue, given_buff_addr[idx] );
    }

    tx_data_available();
    return;
}

// -----------------------------------------------------------------------------+-
// TX SLOTS AVAILABLE
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_CLI_Tx_Slots_Available(void)
{
    uint32_t num_slots = RB_Slots_Available(&client_tx_queue);
    return   num_slots;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// RX API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~

// -----------------------------------------------------------------------------+-
// Register the 'data available' callback;
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Register_Rx_Callback(USART_IT_CLI_Input_Available_Callback given_func_ptr)
{
    input_data_avail = given_func_ptr;
    return;
}

// -----------------------------------------------------------------------------+-
// GET LINE
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_CLI_Get_Line(
    uint8_t  *input_buffer,
    uint32_t  input_buffer_len)
{
    // allow room for the nul terminating char;
    int32_t  available_len = input_buffer_len-1;
    uint32_t idx=0;
    uint8_t  next_byte;

    if(available_len <= 0) {
        // No room in the given buffer;
        input_buffer[0] = '\0';
        return idx;
    }

    if(Cmd_Ready == BUFF_NONE) {
        // No input available;
        input_buffer[0] = '\0';
        return idx;
    }

    PCB_Struct *pcb_ptr = &Double_PCB[Cmd_Ready];

    if(pcb_is_empty(pcb_ptr)) {
        // No input available;
        input_buffer[0] = '\0';
        return idx;
    }

    pcb_reset_get_cmd(pcb_ptr);

    while(idx<available_len) {
        pcb_read_next_char(pcb_ptr, &next_byte);
        input_buffer[idx++] = next_byte;
        if(next_byte == '\n') break;
        if(pcb_is_empty(pcb_ptr)) break;
    }
    input_buffer[idx] = '\0';
    return idx;
}


// =============================================================================================#=
// General Module APIs
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// USART PERIPHERAL INTERRUPT
//
// This should be invoked from the appropriate USART*_IRQHandler.
// -----------------------------------------------------------------------------+-
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

// -----------------------------------------------------------------------------+-
// MODULE INIT
//
// One-time startup initialization for this software module;
//
// USART is connected to the Virtual Com Port
//     USART2
//     TX  PA2
//     RX  PA3
//
// FUTURE TODO: @@@ pass the USART# in as a param and
// save internally for all references to same.
// OR... use board model.
// -----------------------------------------------------------------------------+-
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

