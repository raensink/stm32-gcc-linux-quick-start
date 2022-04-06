
// =============================================================================================#=
// Vector Table
//
// This is a project-specific version of the CMSIS Vector Table.
//
// According to the CMSIS documentation, the CMSIS startup file may require application specific adaptations
// and therefore should be copied from the template directory into the application project folder.
// We have copied and refactored the Vector Table content from the startup file and
// located it here in its own source file.
// It is based on the startup template file from the STM32CubeF0 MCU Package:
//     Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s
//
// DEPENDENCIES:
//     This code assumes the following symbol has been defined elsewhere,
//     typically by the linker script:
//         _estack  Initial value of the SP, typically at the end of RAM.
//
// SPDX-License-Identifier: MIT-0
// =============================================================================================#=
        .syntax  unified     // Use the unified (ARM & THUMB) assembler syntax (ARM Dependent)
        .cpu     cortex-m0   // Specify the target processor. (ARM Dependent)
        .fpu     softvfp     // Specify the floating point format. (ARM Dependent)
        .thumb               // Generate the Thumb instruction set. (ARM Dependent)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Default Handler
//
// All exception and interrupt vectors will point to this function by default.
// Loops forever until: watchdog, debugger, exception, or other interrupt.
// Allows for debugging the source of the unexpected interrupt.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
        .section   .text.Default_Handler, "ax", %progbits   // allow this to be located in its own section.
        .weak      Default_Handler
        .type      Default_Handler, %function   // not sure why the linker cares about this.
        .global    Default_Handler              // why give this symbol external linkage?
Default_Handler:
        b          Default_Handler
        .size      Default_Handler, . - Default_Handler



// -------------------------------------------------------------+-
// Global symbols are made visible to LD and thus
// to other "partial programs" linked with this one.
// Global symbols are given external linkage.
// I am not certain that g_pfnVectors needs to be global;
// it does not appear to be used anywhere else.
// -------------------------------------------------------------+-
.global  g_pfnVectors


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Vector Table Background
// Note that the proper constructs must be placed on this
// to ensure that it ends up at physical address 0x0000.0000.
//
// Exceptions
// An exception is an event which causes deviation from normal processing.
// This includes interrupts, reset, and bus errors.
// A synchronous exception is an exception caused by an instruction.
// A asynchronous exception is an exception not caused by an instruction.
//
// The first 15 exception numbers are System Exceptions defined by the ARMv7 M Architecture.
// CMSIS Core defines standard names for these exception handlers.
//
// Exceptions 16 and beyond are called External Interrupts.
// In this context, "external" refers to the fact that these interrupts originate
// outside of the ARM processor core as defined by the ARM architecture.
// External interrupts are vendor implementation specific.
//
// For each exception handler in the table,
// we define a Weak Aliases to the Default Handler.
// Any function with the same name as the exception handler will
// override this definition of the weak aliase.
//
// See the following for an example of how to do all this in C rather than assembler.
//     mcu/STM32CubeL4/Drivers/CMSIS/NN/NN_Lib_Tests/nn_test/RTE/Device/ARMCM7_SP/startup_ARMCM7.c
//
// TODO: Consider changing g_pfnVectors to __Vectors
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
            .section    .isr_vector, "a", %progbits
            .type       g_pfnVectors, %object
            .size       g_pfnVectors, . -g_pfnVectors
g_pfnVectors:


// -------------------------------------------------------------+-
// Initial Stack Pointer
//
// Hardware will set the value of Main SP to
// this value on reset.
// Consider changing this to __initial_sp
// -------------------------------------------------------------+-
STACKPTR:   .word      _estack

// -------------------------------------------------------------+-
// ARM Cortex-M Built-In Exceptions
// -------------------------------------------------------------+-
EXCPN001:   .word      Reset_Handler

EXCPN002:   .word      NMI_Handler
            .weak      NMI_Handler
            .thumb_set NMI_Handler,         Default_Handler

EXCPN003:   .word      HardFault_Handler
            .weak      HardFault_Handler
            .thumb_set HardFault_Handler,   Default_Handler

EXCPN004:   .word      0
EXCPN005:   .word      0
EXCPN006:   .word      0
EXCPN007:   .word      0
EXCPN008:   .word      0
EXCPN009:   .word      0
EXCPN010:   .word      0

EXCPN011:   .word      SVC_Handler
            .weak      SVC_Handler
            .thumb_set SVC_Handler,          Default_Handler

EXCPN012:   .word      DebugMon_Handler
            .weak      DebugMon_Handler
            .thumb_set DebugMon_Handler,     Default_Handler

            .word      0

EXCPN014:   .word      PendSV_Handler
            .weak      PendSV_Handler
            .thumb_set PendSV_Handler,       Default_Handler

EXCPN015:   .word      SysTick_Handler
            .weak      SysTick_Handler
            .thumb_set SysTick_Handler,      Default_Handler

// -------------------------------------------------------------+-
// External Interrupts
// The ARM Cortex-M core supports interupt lines which are
// 'external' to itself but still within the MCU silicon.
// These interrupts are generated by peripherals that are
// specific to the silicon vendor.
// -------------------------------------------------------------+-
EXCPN016:   .word      WWDG_IRQHandler         @ External Interrupt #0
            .weak      WWDG_IRQHandler
            .thumb_set WWDG_IRQHandler,        Default_Handler

EXCPN017:   .word      PVD_PVM_IRQHandler
            .weak      PVD_PVM_IRQHandler
            .thumb_set PVD_PVM_IRQHandler,     Default_Handler

            .word      TAMP_STAMP_IRQHandler
            .weak      TAMP_STAMP_IRQHandler
            .thumb_set TAMP_STAMP_IRQHandler,  Default_Handler

            .word      RTC_WKUP_IRQHandler
            .weak      RTC_WKUP_IRQHandler
            .thumb_set RTC_WKUP_IRQHandler,    Default_Handler

            .word      FLASH_IRQHandler
            .weak      FLASH_IRQHandler
            .thumb_set FLASH_IRQHandler,       Default_Handler

            .word      RCC_IRQHandler
            .weak      RCC_IRQHandler
            .thumb_set RCC_IRQHandler,         Default_Handler

            .word      EXTI0_IRQHandler
            .weak      EXTI0_IRQHandler
            .thumb_set EXTI0_IRQHandler,       Default_Handler

            .word      EXTI1_IRQHandler
            .weak      EXTI1_IRQHandler
            .thumb_set EXTI1_IRQHandler,       Default_Handler

            .word      EXTI2_IRQHandler
            .weak      EXTI2_IRQHandler
            .thumb_set EXTI2_IRQHandler,       Default_Handler

            .word      EXTI3_IRQHandler
            .weak      EXTI3_IRQHandler
            .thumb_set EXTI3_IRQHandler,       Default_Handler


EXCPN026:   .word      EXTI4_IRQHandler           @ External Interrupt #10
            .weak      EXTI4_IRQHandler
            .thumb_set EXTI4_IRQHandler,          Default_Handler

            .word      DMA1_Channel1_IRQHandler
            .weak      DMA1_Channel1_IRQHandler
            .thumb_set DMA1_Channel1_IRQHandler,  Default_Handler

            .word      DMA1_Channel2_IRQHandler
            .weak      DMA1_Channel2_IRQHandler
            .thumb_set DMA1_Channel2_IRQHandler,  Default_Handler

            .word      DMA1_Channel3_IRQHandler
            .weak      DMA1_Channel3_IRQHandler
            .thumb_set DMA1_Channel3_IRQHandler,  Default_Handler

            .word      DMA1_Channel4_IRQHandler
            .weak      DMA1_Channel4_IRQHandler
            .thumb_set DMA1_Channel4_IRQHandler,  Default_Handler

            .word      DMA1_Channel5_IRQHandler
            .weak      DMA1_Channel5_IRQHandler
            .thumb_set DMA1_Channel5_IRQHandler,  Default_Handler

            .word      DMA1_Channel6_IRQHandler
            .weak      DMA1_Channel6_IRQHandler
            .thumb_set DMA1_Channel6_IRQHandler,  Default_Handler

            .word      DMA1_Channel7_IRQHandler
            .weak      DMA1_Channel7_IRQHandler
            .thumb_set DMA1_Channel7_IRQHandler,  Default_Handler

            .word      ADC1_2_IRQHandler
            .weak      ADC1_2_IRQHandler
            .thumb_set ADC1_2_IRQHandler,         Default_Handler

            .word      CAN1_TX_IRQHandler
            .weak      CAN1_TX_IRQHandler
            .thumb_set CAN1_TX_IRQHandler,        Default_Handler

EXCPN036:   .word      CAN1_RX0_IRQHandler    @ External Interrupt #20
            .weak      CAN1_RX0_IRQHandler
            .thumb_set CAN1_RX0_IRQHandler,    Default_Handler

            .word      CAN1_RX1_IRQHandler
            .weak      CAN1_RX1_IRQHandler
            .thumb_set CAN1_RX1_IRQHandler,    Default_Handler

            .word      CAN1_SCE_IRQHandler
            .weak      CAN1_SCE_IRQHandler
            .thumb_set CAN1_SCE_IRQHandler,    Default_Handler

            .word      EXTI9_5_IRQHandler
            .weak      EXTI9_5_IRQHandler
            .thumb_set EXTI9_5_IRQHandler,    Default_Handler

            .word      TIM1_BRK_TIM15_IRQHandler
            .weak      TIM1_BRK_TIM15_IRQHandler
            .thumb_set TIM1_BRK_TIM15_IRQHandler,    Default_Handler

            .word      TIM1_UP_TIM16_IRQHandler
            .weak      TIM1_UP_TIM16_IRQHandler
            .thumb_set TIM1_UP_TIM16_IRQHandler,    Default_Handler

            .word      TIM1_TRG_COM_TIM17_IRQHandler
            .weak      TIM1_TRG_COM_TIM17_IRQHandler
            .thumb_set TIM1_TRG_COM_TIM17_IRQHandler,    Default_Handler

            .word      TIM1_CC_IRQHandler
            .weak      TIM1_CC_IRQHandler
            .thumb_set TIM1_CC_IRQHandler,    Default_Handler

            .word      TIM2_IRQHandler
            .weak      TIM2_IRQHandler
            .thumb_set TIM2_IRQHandler,    Default_Handler

            .word      TIM3_IRQHandler
            .weak      TIM3_IRQHandler
            .thumb_set TIM3_IRQHandler,    Default_Handler

EXCPN046:   .word      TIM4_IRQHandler     @ External Interrupt #30
            .weak      TIM4_IRQHandler
            .thumb_set TIM4_IRQHandler,    Default_Handler

            .word      I2C1_EV_IRQHandler
            .weak      I2C1_EV_IRQHandler
            .thumb_set I2C1_EV_IRQHandler,    Default_Handler

            .word      I2C1_ER_IRQHandler
            .weak      I2C1_ER_IRQHandler
            .thumb_set I2C1_ER_IRQHandler,    Default_Handler

            .word      I2C2_EV_IRQHandler
            .weak      I2C2_EV_IRQHandler
            .thumb_set I2C2_EV_IRQHandler,    Default_Handler

            .word      I2C2_ER_IRQHandler
            .weak      I2C2_ER_IRQHandler
            .thumb_set I2C2_ER_IRQHandler,    Default_Handler

            .word      SPI1_IRQHandler
            .weak      SPI1_IRQHandler
            .thumb_set SPI1_IRQHandler,    Default_Handler

            .word      SPI2_IRQHandler
            .weak      SPI2_IRQHandler
            .thumb_set SPI2_IRQHandler,    Default_Handler

            .word      USART1_IRQHandler
            .weak      USART1_IRQHandler
            .thumb_set USART1_IRQHandler,    Default_Handler

            .word      USART2_IRQHandler
            .weak      USART2_IRQHandler
            .thumb_set USART2_IRQHandler,    Default_Handler

            .word      USART3_IRQHandler
            .weak      USART3_IRQHandler
            .thumb_set USART3_IRQHandler,    Default_Handler

EXCPN056:   .word      EXTI15_10_IRQHandler     @ External Interrupt #40
            .weak      EXTI15_10_IRQHandler
            .thumb_set EXTI15_10_IRQHandler,   Default_Handler

            .word      RTC_Alarm_IRQHandler

            .word      DFSDM1_FLT3_IRQHandler

            .word      TIM8_BRK_IRQHandler

            .word      TIM8_UP_IRQHandler

            .word      TIM8_TRG_COM_IRQHandler

            .word      TIM8_CC_IRQHandler

            .word      ADC3_IRQHandler

            .word      FMC_IRQHandler

            .word      SDMMC1_IRQHandler

EXCPN066:   .word      TIM5_IRQHandler     @ External Interrupt #50

            .word      SPI3_IRQHandler

            .word      UART4_IRQHandler

            .word      UART5_IRQHandler

            .word      TIM6_DAC_IRQHandler

            .word      TIM7_IRQHandler

            .word      DMA2_Channel1_IRQHandler

            .word      DMA2_Channel2_IRQHandler

            .word      DMA2_Channel3_IRQHandler

            .word      DMA2_Channel4_IRQHandler

EXCPN076:   .word      DMA2_Channel5_IRQHandler     @ External Interrupt #60

            .word      DFSDM1_FLT0_IRQHandler

            .word      DFSDM1_FLT1_IRQHandler

            .word      DFSDM1_FLT2_IRQHandler

            .word      COMP_IRQHandler

            .word      LPTIM1_IRQHandler

            .word      LPTIM2_IRQHandler

            .word      OTG_FS_IRQHandler

            .word      DMA2_Channel6_IRQHandler

            .word      DMA2_Channel7_IRQHandler

EXCPN086:   .word      LPUART1_IRQHandler     @ External Interrupt #70

            .word      QUADSPI_IRQHandler

            .word      I2C3_EV_IRQHandler

            .word      I2C3_ER_IRQHandler

            .word      SAI1_IRQHandler

            .word      SAI2_IRQHandler

            .word      SWPMI1_IRQHandler

            .word      TSC_IRQHandler

            .word      LCD_IRQHandler

            .word      0

EXCPN096:   .word      RNG_IRQHandler     @ External Interrupt #80

EXCPN097:   .word      FPU_IRQHandler     @ External Interrupt #81



            @
            @ TODO: move all of the following statements into
            @ their respective places in the preceding vtor table.
            @
            .weak      RTC_Alarm_IRQHandler
            .thumb_set RTC_Alarm_IRQHandler,    Default_Handler

            .weak      DFSDM1_FLT3_IRQHandler
            .thumb_set DFSDM1_FLT3_IRQHandler,    Default_Handler

            .weak      TIM8_BRK_IRQHandler
            .thumb_set TIM8_BRK_IRQHandler,    Default_Handler

            .weak      TIM8_UP_IRQHandler
            .thumb_set TIM8_UP_IRQHandler,    Default_Handler

            .weak      TIM8_TRG_COM_IRQHandler
            .thumb_set TIM8_TRG_COM_IRQHandler,    Default_Handler

            .weak      TIM8_CC_IRQHandler
            .thumb_set TIM8_CC_IRQHandler,    Default_Handler

            .weak      ADC3_IRQHandler
            .thumb_set ADC3_IRQHandler,    Default_Handler

            .weak      FMC_IRQHandler
            .thumb_set FMC_IRQHandler,    Default_Handler

            .weak      SDMMC1_IRQHandler
            .thumb_set SDMMC1_IRQHandler,    Default_Handler

            .weak      TIM5_IRQHandler
            .thumb_set TIM5_IRQHandler,    Default_Handler

            .weak      SPI3_IRQHandler
            .thumb_set SPI3_IRQHandler,    Default_Handler

            .weak      UART4_IRQHandler
            .thumb_set UART4_IRQHandler,    Default_Handler

            .weak      UART5_IRQHandler
            .thumb_set UART5_IRQHandler,    Default_Handler

            .weak      TIM6_DAC_IRQHandler
            .thumb_set TIM6_DAC_IRQHandler,    Default_Handler

            .weak      TIM7_IRQHandler
            .thumb_set TIM7_IRQHandler,    Default_Handler

            .weak      DMA2_Channel1_IRQHandler
            .thumb_set DMA2_Channel1_IRQHandler,    Default_Handler

            .weak      DMA2_Channel2_IRQHandler
            .thumb_set DMA2_Channel2_IRQHandler,    Default_Handler

            .weak      DMA2_Channel3_IRQHandler
            .thumb_set DMA2_Channel3_IRQHandler,    Default_Handler

            .weak      DMA2_Channel4_IRQHandler
            .thumb_set DMA2_Channel4_IRQHandler,    Default_Handler

            .weak      DMA2_Channel5_IRQHandler
            .thumb_set DMA2_Channel5_IRQHandler,    Default_Handler

            .weak      DFSDM1_FLT0_IRQHandler
            .thumb_set DFSDM1_FLT0_IRQHandler,    Default_Handler

            .weak      DFSDM1_FLT1_IRQHandler
            .thumb_set DFSDM1_FLT1_IRQHandler,    Default_Handler

            .weak      DFSDM1_FLT2_IRQHandler
            .thumb_set DFSDM1_FLT2_IRQHandler,    Default_Handler

            .weak      COMP_IRQHandler
            .thumb_set COMP_IRQHandler,    Default_Handler

            .weak      LPTIM1_IRQHandler
            .thumb_set LPTIM1_IRQHandler,    Default_Handler

            .weak      LPTIM2_IRQHandler
            .thumb_set LPTIM2_IRQHandler,    Default_Handler

            .weak      OTG_FS_IRQHandler
            .thumb_set OTG_FS_IRQHandler,    Default_Handler

            .weak      DMA2_Channel6_IRQHandler
            .thumb_set DMA2_Channel6_IRQHandler,    Default_Handler

            .weak      DMA2_Channel7_IRQHandler
            .thumb_set DMA2_Channel7_IRQHandler,    Default_Handler

            .weak      LPUART1_IRQHandler
            .thumb_set LPUART1_IRQHandler,    Default_Handler

            .weak      QUADSPI_IRQHandler
            .thumb_set QUADSPI_IRQHandler,    Default_Handler

            .weak      I2C3_EV_IRQHandler
            .thumb_set I2C3_EV_IRQHandler,    Default_Handler

            .weak      I2C3_ER_IRQHandler
            .thumb_set I2C3_ER_IRQHandler,    Default_Handler

            .weak      SAI1_IRQHandler
            .thumb_set SAI1_IRQHandler,    Default_Handler

            .weak      SAI2_IRQHandler
            .thumb_set SAI2_IRQHandler,    Default_Handler

            .weak      SWPMI1_IRQHandler
            .thumb_set SWPMI1_IRQHandler,    Default_Handler

            .weak      TSC_IRQHandler
            .thumb_set TSC_IRQHandler,    Default_Handler

            .weak      LCD_IRQHandler
            .thumb_set LCD_IRQHandler,    Default_Handler

            .weak      RNG_IRQHandler
            .thumb_set RNG_IRQHandler,    Default_Handler

            .weak      FPU_IRQHandler
            .thumb_set FPU_IRQHandler,    Default_Handler

