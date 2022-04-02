
/*
================================================================================================#=
Vector Table

This is a project-specific version of the CMSIS Vector Table.
It is based on the startup file from the STM32CubeL4 MCU Package:
    Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s
According to the CMSIS documentation, the CMSIS startup file may require application specific adaptations
and therefore should be copied from the template directory into the application project folder.
We have copied and refactored the Vector Table content from the startup file and
located it here in its own source file.

DEPENDENCIES:
    This code assumes the following symbols have been defined elsewhere,
    typically by the linker script:

    _estack  Initial value of the SP, typically at the end of RAM.
================================================================================================#=
*/
        .syntax  unified     @ Use the unified (ARM & THUMB) assembler syntax (ARM Dependent)
        .cpu     cortex-m0   @ Specify the target processor. (ARM Dependent)
        .fpu     softvfp     @ Specify the floating point format. (ARM Dependent)
        .thumb               @ Generate the Thumb instruction set. (ARM Dependent)


/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
Default Handler: Spin Forever

All exception and interrupt vectors will point to this function by default.
Loops forever until: watchdog, debugger, exception, or other interrupt.
Allows for debugging the source of the unexpected interrupt.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
        .section   .text.Default_Handler, "ax", %progbits   @ allow this to be located in its own section.
        .weak      Spin_Forever
        .type      Spin_Forever, %function   @ not sure why the linker cares about this.
        .global    Spin_Forever              @ why give this symbol external linkage?
Spin_Forever:
        b          Spin_Forever
        .size      Spin_Forever, . - Spin_Forever




@ Global symbols are made visible to LD and thus to other "partial programs" linked with this one.
@ Global symbols are given external linkage.
@ I am not certain that g_pfnVectors needs to be global - does not appear to be used anywhere else.
.global  g_pfnVectors








/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
Vector Table for Cortex-M4.
Note that the proper constructs must be placed on this
to ensure that it ends up at physical address 0x0000.0000.

See the following for an example of how to do this in C rather than assembler.
mcu/STM32CubeL4/Drivers/CMSIS/NN/NN_Lib_Tests/nn_test/RTE/Device/ARMCM7_SP/startup_ARMCM7.c

Exceptions
An exception is an event which causes deviation from normal processing.
This includes interrupts, reset, and bus errors.
A synchronous exception is an exception caused by an instruction.
A asynchronous exception is an exception not caused by an instruction.

The first 15 exception numbers are System Exceptions defined by the ARMv7 M Architecture.
CMSIS Core defines standard names for these exception handlers.

Exceptions 16 and beyond are called External Interrupts.
In this context, "external" refers to the fact that these interrupts originate
outside of the ARM processor core as defined by the ARM architecture.
External interrupts are vendor implementation specific.

For each exception handler in the table,
we define a Weak Aliases to the Default Handler.
Any function with the same name as the exception handler will
override this definition of the weak aliase.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
*/
            .section    .isr_vector, "a", %progbits
            .type       g_pfnVectors, %object
            .size       g_pfnVectors, . -g_pfnVectors

g_pfnVectors:                         @ Change this to __Vectors
STACKPTR:   .word      _estack        @ Hardware will set the value of Main SP to this value on reset.   Change this to __initial_sp

EXCPN001:   .word      Reset_Handler  @ Defined elsewhere.

EXCPN002:   .word      NMI_Handler
            .weak      NMI_Handler
            .thumb_set NMI_Handler,         Spin_Forever

EXCPN003:   .word      HardFault_Handler
            .weak      HardFault_Handler
            .thumb_set HardFault_Handler,   Spin_Forever

EXCPN004:   .word      0
EXCPN005:   .word      0
EXCPN006:   .word      0
EXCPN007:   .word      0
EXCPN008:   .word      0
EXCPN009:   .word      0
EXCPN010:   .word      0

EXCPN011:   .word      SVC_Handler
            .weak      SVC_Handler
            .thumb_set SVC_Handler,          Spin_Forever

            .word  0
            .word  0

EXCPN014:   .word      PendSV_Handler
            .weak      PendSV_Handler
            .thumb_set PendSV_Handler,       Spin_Forever

EXCPN015:   .word      SysTick_Handler
            .weak      SysTick_Handler
            .thumb_set SysTick_Handler,      Spin_Forever

EXCPN016:   .word      WWDG_IRQHandler       /* Window WatchDog              */
            .weak      WWDG_IRQHandler
            .thumb_set WWDG_IRQHandler,      Spin_Forever

            .word      PVD_VDDIO2_IRQHandler             /* PVD and VDDIO2 through EXTI Line detect */
            .weak      PVD_VDDIO2_IRQHandler
            .thumb_set PVD_VDDIO2_IRQHandler,         Spin_Forever

            .word      RTC_IRQHandler                    /* RTC through the EXTI line    */
            .weak      RTC_IRQHandler
            .thumb_set RTC_IRQHandler,         Spin_Forever

            .word      FLASH_IRQHandler                  /* FLASH                        */
            .weak      FLASH_IRQHandler
            .thumb_set FLASH_IRQHandler,         Spin_Forever

            .word      RCC_CRS_IRQHandler                /* RCC and CRS                  */
            .weak      RCC_CRS_IRQHandler
            .thumb_set RCC_CRS_IRQHandler,         Spin_Forever

            .word      EXTI0_1_IRQHandler                /* EXTI Line 0 and 1            */
            .weak      EXTI0_1_IRQHandler
            .thumb_set EXTI0_1_IRQHandler,         Spin_Forever

            .word      EXTI2_3_IRQHandler                /* EXTI Line 2 and 3            */
            .weak      EXTI2_3_IRQHandler
            .thumb_set EXTI2_3_IRQHandler,         Spin_Forever

            .word      EXTI4_15_IRQHandler               /* EXTI Line 4 to 15            */
            .weak      EXTI4_15_IRQHandler
            .thumb_set EXTI4_15_IRQHandler,         Spin_Forever

            .word      TSC_IRQHandler                    /* TSC                          */
            .weak      TSC_IRQHandler
            .thumb_set TSC_IRQHandler,         Spin_Forever

            .word      DMA1_Ch1_IRQHandler               /* DMA1 Channel 1               */
            .weak      DMA1_Ch1_IRQHandler
            .thumb_set DMA1_Ch1_IRQHandler,         Spin_Forever

            .word      DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler  /* DMA1 Channel 2 and 3 & DMA2 Channel 1 and 2 */
            .weak      DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler
            .thumb_set DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler,         Spin_Forever

            .word      DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler  /* DMA1 Channel 4 to 7 & DMA2 Channel 3 to 5 */
            .weak      DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler
            .thumb_set DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler,         Spin_Forever

            .word      ADC1_COMP_IRQHandler              /* ADC1, COMP1 and COMP2         */
            .weak      ADC1_COMP_IRQHandler
            .thumb_set ADC1_COMP_IRQHandler,         Spin_Forever

            .word      TIM1_BRK_UP_TRG_COM_IRQHandler    /* TIM1 Break, Update, Trigger and Commutation */
            .weak      TIM1_BRK_UP_TRG_COM_IRQHandler
            .thumb_set TIM1_BRK_UP_TRG_COM_IRQHandler,         Spin_Forever

            .word      TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */
            .weak      TIM1_CC_IRQHandler
            .thumb_set TIM1_CC_IRQHandler,         Spin_Forever

            .word      TIM2_IRQHandler                   /* TIM2                         */
            .weak      TIM2_IRQHandler
            .thumb_set TIM2_IRQHandler,         Spin_Forever

            .word      TIM3_IRQHandler                   /* TIM3                         */
            .weak      TIM3_IRQHandler
            .thumb_set TIM3_IRQHandler,         Spin_Forever

            .word      TIM6_DAC_IRQHandler               /* TIM6 and DAC                 */
            .weak      TIM6_DAC_IRQHandler
            .thumb_set TIM6_DAC_IRQHandler,         Spin_Forever

            .word      TIM7_IRQHandler                   /* TIM7                         */
            .weak      TIM7_IRQHandler
            .thumb_set TIM7_IRQHandler,         Spin_Forever

            .word      TIM14_IRQHandler                  /* TIM14                        */
            .weak      TIM14_IRQHandler
            .thumb_set TIM14_IRQHandler,         Spin_Forever

            .word      TIM15_IRQHandler                  /* TIM15                        */
            .weak      TIM15_IRQHandler
            .thumb_set TIM15_IRQHandler,         Spin_Forever

            .word      TIM16_IRQHandler                  /* TIM16                        */
            .weak      TIM16_IRQHandler
            .thumb_set TIM16_IRQHandler,         Spin_Forever

            .word      TIM17_IRQHandler                  /* TIM17                        */
            .weak      TIM17_IRQHandler
            .thumb_set TIM17_IRQHandler,         Spin_Forever

            .word      I2C1_IRQHandler                   /* I2C1                         */
            .weak      I2C1_IRQHandler
            .thumb_set I2C1_IRQHandler,         Spin_Forever

            .word      I2C2_IRQHandler                   /* I2C2                         */
            .weak      I2C2_IRQHandler
            .thumb_set I2C2_IRQHandler,         Spin_Forever

            .word      SPI1_IRQHandler                   /* SPI1                         */
            .weak      SPI1_IRQHandler
            .thumb_set SPI1_IRQHandler,         Spin_Forever

            .word      SPI2_IRQHandler                   /* SPI2                         */
            .weak      SPI2_IRQHandler
            .thumb_set SPI2_IRQHandler,         Spin_Forever

            .word      USART1_IRQHandler                 /* USART1                       */
            .weak      USART1_IRQHandler
            .thumb_set USART1_IRQHandler,         Spin_Forever

            .word      USART2_IRQHandler                 /* USART2                       */
            .weak      USART2_IRQHandler
            .thumb_set USART2_IRQHandler,         Spin_Forever

            .word      USART3_8_IRQHandler               /* USART3, USART4, USART5, USART6, USART7, USART8 */
            .weak      USART3_8_IRQHandler
            .thumb_set USART3_8_IRQHandler,         Spin_Forever

            .word      CEC_CAN_IRQHandler                /* CEC and CAN                  */
            .weak      CEC_CAN_IRQHandler
            .thumb_set CEC_CAN_IRQHandler,         Spin_Forever


