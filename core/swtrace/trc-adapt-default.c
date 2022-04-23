
/*
================================================================================================#=
TRACE CORE ADAPTATION - DEFAULT IMPLEMENTATION
core\swtrace\trc-adapt-default.c

Description:
    A default set of adaptations that implement the adaptation API.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "core/swtrace/trc-adaptation.h"


#include "platform/usart/usart-it-cli.h"



// ---------------------------------------------------------------------+-
bool TRC_Dispatch_Message(uint8_t *given_msg, uint32_t msg_len)
{
    if(USART_IT_CLI_Trace_Slots_Available() < msg_len) return false;

    USART_IT_CLI_Put_Trace(given_msg, msg_len);

    return true;
};


// ---------------------------------------------------------------------+-
// Default implementation of the Adaptation Init API
// ---------------------------------------------------------------------+-
void TRC_Adapt_Init(void)
{
    return;
};





#if 0
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
/*
================================================================================================#=
Dependencies:

================================================================================================#=
*/

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "client-api.h"
#include "platform-devices.h"
#include "platform/usart/usart-it-cli.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Private Data and Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
#define WAIT_FOR_MUTEX_TIMEOUT_MS (200U)

// Protects critical sections inside tlog module to
// allow multiple tasks to concurrently access tLog services.
static SemaphoreHandle_t  tlogMutex;

// This is the "device" to which we deliver all trace output.
//static TTBIO_device_handle  ttbioSwTraceDeviceHandle;

// @@@ Future: log this value out during clean shutdown;
//static uint32_t outputRetries  = 0;
//static uint32_t outputFailures = 0;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Module Internal Functions
// Not intended for clients outside this module.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~



// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
uint32_t tlog_get_current_ticks(void)
{
    return xTaskGetTickCount();
};


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
bool tlog_enter_critical_section(void)
{
    if( pdTRUE == xSemaphoreTake(tlogMutex, (TickType_t)WAIT_FOR_MUTEX_TIMEOUT_MS) )
    {
        return true;
    }
    else
    {
        return false;
    }
};


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
void tlog_exit_critical_section(void)
{
    if( pdFALSE == xSemaphoreGive(tlogMutex) )
    {
        // this should not happen
        // perhaps assert here
    }
    return;
};


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
void tlog_handle_assertion_failure(void)
{
    // @@@@ It seems this only halts the current task;
    // in production, this should reset the processor?
    assert(false);
};


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
bool tlog_output_formatted_trace_message(
    uint8_t  *givenMessageBuff,
    uint32_t  givenMessageLen )
{
     USART_IT_CLI_Put_Trace(givenMessageBuff, givenMessageLen);
     return true;
}
#endif
