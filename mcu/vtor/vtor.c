
/*
================================================================================================#=
This module contains a project-specific adaptation of the CMSIS SystemInit function.
It is based on the system_<device>.c file from the STM32CubeL4 MCU Package:
    Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s

According to the CMSIS documentation, the CMSIS system file may require application specific
adaptations and therefore should be copied into the application project folder.
We have done this, in part, by refactoring the vector table location setup code
and putting into its own module here.

TODO: @@@@@ ReFactor and Thoroughly Comment
NOTE: this is file is built but is not currently used.
================================================================================================#=
*/

#include "mcu/vtor/vtor.h"
#include "stm32l4xx.h"

// @@@ TODO - move this VTOR Table stuff to a new VTOR module that provides a service to manage this.
// @@@ TODO - move this VTOR Table stuff to a new VTOR module that provides a service to manage this.
// @@@ TODO - move this VTOR Table stuff to a new VTOR module that provides a service to manage this.


/* Note: Following vector table addresses must be defined in line with linker configuration. */
/*!< Uncomment the following line if you need to relocate the vector table
     anywhere in Flash or Sram, else the vector table is kept at the automatic remap of boot address selected */

/* #define USER_VECT_TAB_ADDRESS */
#if defined(USER_VECT_TAB_ADDRESS)


/*!< Uncomment the following line if you need to relocate your vector Table
     in Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */

#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM1_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS   FLASH_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#endif /* VECT_TAB_SRAM */
#endif /* USER_VECT_TAB_ADDRESS */


// @@@@@@@@@@@@@@@@@@@ REFACTOR & IMPROVE THIS @@@@@@@@@@@@@@@@@@@@@@@@
// =============================================================================================#=
// Set Vector Table Location
// according to the #defines above.
// =============================================================================================#=
void CMSIS_VTOR_SetVectorTableLocation(void)
{

// @@@ TODO @@@
// Refactor and move this VTOR Table stuff to a new VTOR module that provides a service to manage this.
#if defined(USER_VECT_TAB_ADDRESS)
    // Configure the Vector Table location
    SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET;
#endif

    return;
}

