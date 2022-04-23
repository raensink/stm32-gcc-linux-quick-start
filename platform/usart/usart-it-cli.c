
// =============================================================================================#=
// USART INTERRUPT DRIVEN CLI API
// platform/usart/usart-it-cli.c
//
// TODO: if you slam too many chars into the command line all at once, things go splat somewhere.
//
// SPDX-License-Identifier: MIT-0
// =============================================================================================#=

#include "platform/usart/usart-it-cli.h"

#include <ctype.h>
#include <string.h>


// Project Dependencies
#include "platform/util/ring-buffer.h"

#include "core/swtrace/trc-led.h"


// STM32 Low Level Drivers
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_bus.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_rcc.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_gpio.h"
#include "STM32L4xx_HAL_Driver/Inc/stm32l4xx_ll_usart.h"



// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// Internal Ring Buffers
// Size must be a power of two;
// -----------------------------------------------------------------------------+-
static uint8_t input_buffer[64];      // RX chars from the user's terminal;
static uint8_t echo_buffer[64];       // TX chars to be echo'd back to the user;
static uint8_t trace_buffer[1024];    // TX chars from the software trace facility;
static uint8_t response_buffer[512];  // TX response chars from command processing;

static Ring_Buffer input_rb = {
    .buff = input_buffer,
    .size = sizeof(input_buffer),
    .tail = 0,
    .head = 0,
};

static Ring_Buffer echo_rb = {
    .buff = echo_buffer,
    .size = sizeof(echo_buffer),
    .tail = 0,
    .head = 0,
};

static Ring_Buffer trace_rb = {
    .buff = trace_buffer,
    .size = sizeof(trace_buffer),
    .tail = 0,
    .head = 0,
};

static Ring_Buffer response_rb = {
    .buff = response_buffer,
    .size = sizeof(response_buffer),
    .tail = 0,
    .head = 0,
};


// -----------------------------------------------------------------------------+-
// This is how we prompt the user.
// -----------------------------------------------------------------------------+-
static char *cli_prompt = "CLI > ";

// -----------------------------------------------------------------------------+-
// Callback for 'input data available' as
// registered by the client application layer.
// This is called when there is a command line ready
// for consumption by the client application layer.
// -----------------------------------------------------------------------------+-
USART_IT_CLI_Input_Available_Callback input_data_avail = NULL;

// -----------------------------------------------------------------------------+-
// Set true when the user's command line has been
// stomped upon by any non-echo character output;
// -----------------------------------------------------------------------------+-
static bool restore_user_cmd_line = true;

// -----------------------------------------------------------------------------+-
// XON / XOFF
// This flag supports in-band software flow control using CTRL+S and CTRL+Q;
//
// Setting XON to false will stop the software trace output, but
// still allow command response output
//
// Setting XON back to true will return the CLI to normal output.
// -----------------------------------------------------------------------------+-
static bool XON = true;

// -----------------------------------------------------------------------------+-
// PENDING COMMAND BUFFER (PCB)
// This is where we build up the user's command line before
// it is delivered up to the application client.
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

// -------------------------------------------------------------+-
// Keep track of which buffer is pending/in-progress
// and which, if any, is ready to be consumed.
// -------------------------------------------------------------+-
#define BUFF_NONE     (-1)
#define BUFF_A         (0)
#define BUFF_B         (1)
static int8_t  Cmd_In_Progress = BUFF_A;
static int8_t  Cmd_Ready       = BUFF_NONE;


// -------------------------------------------------------------+-
// Keep a few metrics for future troubleshooting.
// Future: trace log these to the console during clean shutdown.
// -------------------------------------------------------------+-
static uint32_t input_rb_overflow     = 0;
static uint32_t echo_rb_overflow      = 0;
static uint32_t trace_rb_overflow     = 0;
static uint32_t response_rb_overflow  = 0;




// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=

// -----------------------------------------------------------------------------+-
// PCB Accessor Helper Functions
// -----------------------------------------------------------------------------+-
static void pcb_reset(PCB_Struct *pcb)
{
    pcb->read_idx = 0;
    pcb->tail_idx = 0;
}

static void pcb_reset_read_idx(PCB_Struct *pcb)
{
    pcb->read_idx = 0;
}

static bool pcb_is_full(PCB_Struct *pcb)
{
    // allow one byte for the terminating newline delimiter;
    return(pcb->tail_idx >= pcb->size - 1);
}

static uint32_t pcb_strlen(PCB_Struct *pcb)
{
    return(pcb->tail_idx);
}

static bool pcb_is_empty(PCB_Struct *pcb)
{
    return(pcb->tail_idx == 0);
}

static void pcb_add_char(
    PCB_Struct *pcb,
    uint8_t     new_byte)
{
    pcb->buff[pcb->tail_idx] = new_byte;
    pcb->tail_idx++;
}

static void pcb_erase_last_char(PCB_Struct *pcb)
{
    if(pcb->tail_idx > 0) pcb->tail_idx--;
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
// Helper function to signal to the USART peripheral hardware
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
// -----------------------------------------------------------------------------+-
static void echo_this_char_to_terminal(uint8_t given_char)
{
    if(RB_Is_Full(&echo_rb)) {
        echo_rb_overflow++;
    }
    else {
        RB_Write_Byte_To_Tail(&echo_rb, given_char);
    }
}

// -----------------------------------------------------------------------------+-
// -----------------------------------------------------------------------------+-
static void echo_cli_prompt_to_terminal(void)
{
    for( int c=0; c < strlen(cli_prompt); c++)
    {
        echo_this_char_to_terminal(cli_prompt[c]);
    }
}

// -----------------------------------------------------------------------------+-
// -----------------------------------------------------------------------------+-
static void echo_pending_chars_to_terminal(PCB_Struct *pcb_ptr)
{
    uint8_t pcb_char;

    pcb_reset_read_idx(pcb_ptr);
    while(pcb_read_next_char(pcb_ptr, &pcb_char)) {
        echo_this_char_to_terminal(pcb_char);
    }
}


// -----------------------------------------------------------------------------+-
// Helper function to process an incoming character from the user's terminal;
// -----------------------------------------------------------------------------+-
static void process_input_char(uint8_t given_char)
{
    // All new chars get added to the PCB in progress;
    PCB_Struct *pcb_ptr = &Double_PCB[Cmd_In_Progress];

    // -----------------------------------------------------------------------------+-
    // Restablish the user's command line, as needed.
    //
    // This function is called only when there is some input character
    // to be processed. When the user's command line needs to be restored,
    // we'll use that incoming character as our trigger to do so.
    // However, when that trigger is an enter key, we let the normal input
    // processing below handle the restoration of the prompt.
    // -----------------------------------------------------------------------------+-
    if(restore_user_cmd_line && given_char != '\r') {
        echo_this_char_to_terminal('\r');

        echo_cli_prompt_to_terminal();
        echo_pending_chars_to_terminal(pcb_ptr);

        restore_user_cmd_line = false;
    }

    // -----------------------------------------------------------------------------+-
    // Process the input char;
    // -----------------------------------------------------------------------------+-
    if(pcb_is_full(pcb_ptr)) {
        // Treat this as if the last character entered was a CR;
        given_char = '\r';
    }

    // -----------------------------------------------------+-
    // Check for and process the 'Enter' key;
    // -----------------------------------------------------+-
    if(given_char == '\r') {

        if(pcb_strlen(pcb_ptr) > 0) {
            // There is a non-empty command ready to be consumed;
            // Add the EOL delimiter to the end of the PCB
            // and notify the client;

            // Note: around here, we prefer our EOL delimiters as newlines;
            pcb_add_char(pcb_ptr, '\n');
            echo_this_char_to_terminal(given_char);

            Cmd_Ready = Cmd_In_Progress;

            // INVOKE CLIENT CALLBACK! (if there is one)
            if(input_data_avail != NULL) {
                input_data_avail( pcb_strlen(pcb_ptr) );
            }

            // Swap the double buffs and reset the buff newly in-progress;
            Cmd_In_Progress = (Cmd_Ready == BUFF_A) ? BUFF_B : BUFF_A;
            pcb_reset(&Double_PCB[Cmd_In_Progress]);
            pcb_ptr = &Double_PCB[Cmd_In_Progress];
        }
        else {
            // Empty command line,
            // just prompt the user again and
            // do not add the CR to the PCB;
            echo_this_char_to_terminal('\n');
            echo_cli_prompt_to_terminal();
        }
    }

    // -----------------------------------------------------+-
    // The ESC key is our recommended method for the user
    // to restore their command line without having to
    // enter an unnecessary printable character.
    // -----------------------------------------------------+-
    else if(given_char == '\e') {
        // In reality, ESC is treated just like
        // any other non-printable character;
    }

    // -----------------------------------------------------+-
    // C-s will suspend all trace output to the CLI;
    // C-q will resume trace output;
    // -----------------------------------------------------+-
    else if(given_char == '\x13') {
        XON = false; // Control S
    }
    else if(given_char == '\x11') {
        XON = true; // Control Q
    }

    // -----------------------------------------------------+-
    // Backspace character;
    // So far, only CNTRL+H seems to work
    // -----------------------------------------------------+-
    else if(
        given_char == '\b' ||
        given_char == '\x8' ||
        given_char == '\127')
    {
        Trace_Blue_Toggle();
        pcb_erase_last_char(pcb_ptr);

        // Send backspace char;
        echo_this_char_to_terminal('\b');

        // Send escape code to clear line from cursor right;
        echo_this_char_to_terminal('\x1b');
        echo_this_char_to_terminal('[');
        echo_this_char_to_terminal('K');
    }

    // -----------------------------------------------------+-
    // Otherwise, filter out any non printable chars;
    // -----------------------------------------------------+-
    else if(!isprint(given_char)) {
        // Filter out any non printable character by doing nothing here.
    }

    // -----------------------------------------------------+-
    // perhaps more filtering is needed?
    // -----------------------------------------------------+-
    // -----------------------------------------------------+-
    // Otherwise, add any other char to the PCB and
    // echo it back to the terminal;
    // -----------------------------------------------------+-
    else {
        pcb_add_char(pcb_ptr, given_char);
        echo_this_char_to_terminal(given_char);
    }
}


// -----------------------------------------------------------------------------+-
// Helper function to do the needful when the USART TDR is empty;
// NOTICE: this is called by the USART IRQ Handler;
//
// Also note: any write to the TDR will clear
// the TXE bit in the USART peripheral;
// -----------------------------------------------------------------------------+-
static void usart_tdr_empty(void)
{
    uint8_t  next_char;
    bool     write_tdr = false;

    static bool  CR_Needed            = false;
    static bool  response_in_progress = false;
    static bool  trace_in_progress    = false;
    static bool  echo_in_progress     = false;

    if(RB_Is_Empty(&response_rb)) response_in_progress = false;
    if(RB_Is_Empty(&trace_rb))    trace_in_progress    = false;
    if(RB_Is_Empty(&echo_rb))     echo_in_progress     = false;

    // -------------------------------------------------------------+-
    // Exhaustively consume any queue we've started reading;
    // -------------------------------------------------------------+-
    if(CR_Needed) {
        next_char = '\r';
        write_tdr = true;
        CR_Needed = false;
    }
    else if(response_in_progress) {
        next_char = RB_Read_Byte_From_Head(&response_rb);
        write_tdr = true;
        restore_user_cmd_line = true;
    }
    else if(trace_in_progress) {
        next_char = RB_Read_Byte_From_Head(&trace_rb);
        write_tdr = true;
        restore_user_cmd_line = true;
    }
    else if(echo_in_progress) {
        next_char = RB_Read_Byte_From_Head(&echo_rb);
        write_tdr = true;
    }
    else {
        // -------------------------------------------------------------+-
        // Otherwise process any new input characters;
        // Input chars from the terminal are
        // added to the PCB and sent to the echo queue;
        // -------------------------------------------------------------+-
        while(RB_Is_Not_Empty(&input_rb)) {
            next_char = RB_Read_Byte_From_Head(&input_rb);
            process_input_char(next_char);
        }

        // -------------------------------------------------------------+-
        // And then pick a new queue to start consuming;
        // We only start reading from the trace log
        // if XON is enabled;
        // -------------------------------------------------------------+-
        if(RB_Is_Not_Empty(&echo_rb)) {
            next_char = RB_Read_Byte_From_Head(&echo_rb);
            write_tdr = true;
            echo_in_progress = true;
        }
        else if(RB_Is_Not_Empty(&response_rb)) {
            next_char = RB_Read_Byte_From_Head(&response_rb);
            write_tdr = true;
            restore_user_cmd_line = true;
            response_in_progress = true;
        }
        else if(XON && RB_Is_Not_Empty(&trace_rb)) {
            next_char = RB_Read_Byte_From_Head(&trace_rb);
            write_tdr = true;
            restore_user_cmd_line = true;
            trace_in_progress = true;
        }
    }

    if(write_tdr) {
        LL_USART_TransmitData8(USART2, next_char);
        if(next_char == '\n') CR_Needed = true;
    }
    else {
        LL_USART_DisableIT_TXE(USART2);
    }
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

    if(RB_Is_Full(&input_rb)) {
        // All we can do is throw the byte away;
        input_rb_overflow++;
    }
    else {
        RB_Write_Byte_To_Tail( &input_rb, rx_byte );
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
// PUT STRING BEST EFFORT
//
// Write the given content into the appropriate ring buffer;
// -----------------------------------------------------------------------------+-
void USART_IT_CLI_Put_Response(uint8_t *given_buff_addr, uint8_t given_buff_len)
{
    uint32_t num_slots = RB_Slots_Available(&response_rb);

    if (num_slots < given_buff_len) {
        response_rb_overflow++;
        return;
    }
    for(int idx=0; idx<given_buff_len; idx++) {
        RB_Write_Byte_To_Tail( &response_rb, given_buff_addr[idx] );
    }
    tx_data_available();
    return;
}

void USART_IT_CLI_Put_Trace(uint8_t *given_buff_addr, uint8_t given_buff_len)
{
    uint32_t num_slots = RB_Slots_Available(&trace_rb);

    if (num_slots < given_buff_len) {
        trace_rb_overflow++;
        return;
    }
    for(int idx=0; idx<given_buff_len; idx++) {
        RB_Write_Byte_To_Tail( &trace_rb, given_buff_addr[idx] );
    }
    tx_data_available();
    return;
}

// -----------------------------------------------------------------------------+-
// SLOTS AVAILABLE
// -----------------------------------------------------------------------------+-
uint32_t USART_IT_CLI_Response_Slots_Available(void)
{
    return RB_Slots_Available(&response_rb);
}

uint32_t USART_IT_CLI_Trace_Slots_Available(void)
{
    return RB_Slots_Available(&trace_rb);
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

    pcb_reset_read_idx(pcb_ptr);

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

// Reserved for debug;
// Trace_Red_Toggle();
// Trace_Blue_Toggle();
// Trace_Red_On();
// Trace_Red_Off();

