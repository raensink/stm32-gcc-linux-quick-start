#pragma once

/*
================================================================================================#=
TRACE CORE
core\swtrace\trc-core.h

Description:
    Defines the API into the software trace log core.

    A client application would include this file only when it needs
    to manage the trace core to, for example, call the init function.
    This include file is not necessary for using the common trace facilities.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include "trc.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// One-time startup initialization for the module.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
extern void TRC_Initialize(void);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Set trace level such that only those messages at or above
// the given level are logged to the backend.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
extern void TRC_SetLogLevel(trcLvl given_level);


