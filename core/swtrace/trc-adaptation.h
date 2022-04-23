#pragma once

/*
================================================================================================#=
SOFTWARE TRACE ADAPTATION API
core/swtrace/trc-adaptation.h

Description:
    Defines the API to the trace adaptation sub-module.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdbool.h>
#include <stdint.h>



// ---------------------------------------------------------------------+-
// A function to do the needful with the given trace message.
// It's up to the adaptation to decide what that means
// but, typically, it means writing the message to a local serial port.
// 
// This should be implemented by
// an adaptation sub-module suitable for the target platform.
// ---------------------------------------------------------------------+-
bool TRC_Dispatch_Message(
    uint8_t  *trace_message,
    uint32_t  trace_messageLen );


// ---------------------------------------------------------------------+-
// ---------------------------------------------------------------------+-
void TRC_Adapt_Init(void);





