
/*
================================================================================================#=
MCU Core System Control Block (SCB)

The purpose of this module is to provide SCB startup initialization services.
At this time there is only one such service and that is to enable the coprocessor.
================================================================================================#=
*/

#pragma once

// If the build is configured to do so, enable the Cortex M4 FPU
void MCUCORE_SCB_MaybeEnableFPU(void);

