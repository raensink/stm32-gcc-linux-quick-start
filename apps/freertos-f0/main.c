
/*
================================================================================================#=
MAIN
apps/freertos/main.c

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "CMSIS/Device/ST/STM32F0xx/Include/stm32f091xc.h"

#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_bus.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_system.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_rcc.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_gpio.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_exti.h"
#include "STM32F0xx_HAL_Driver/Inc/stm32f0xx_ll_utils.h"



// =============================================================================================#=
// Private Internal Types and Data
// =============================================================================================#=

// Values passed to the two tasks just to check the task parameter functionality.
#define mainQUEUE_SEND_PARAMETER      ( 0x1111UL )
#define mainQUEUE_RECEIVE_PARAMETER   ( 0xA5A5UL )

/* Priorities at which the tasks are created. */
#define  mainQUEUE_RECEIVE_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )
#define  mainQUEUE_SEND_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_PERIOD_MS constant. */
#define  mainQUEUE_SEND_FREQUENCY_MS        ( 200 / portTICK_PERIOD_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define  mainQUEUE_LENGTH        ( 1 )


// -----------------------------------------------------------------------------+-
// -----------------------------------------------------------------------------+-
static void init_led_gpio(void)
{
    // -----------------------------------------------------------------------------+-
    // GPIO PA5;
    // On-Board Green User LED (LD2)
    //
    // We know from the UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
    // that the on-board green user LED (LD2) is connected to 'Arduino' Pin D13;
    // See Figure 3 and Section 6.4;
    //
    // We also know from Table 23 on page 52
    // that D13 is connected to GPIO PA5 on the MCU;
    // -----------------------------------------------------------------------------+-

    // Enable AHB2 Clock to GPIO Port A in the Reset and Clock Control peripheral;
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    // Configure pin for output push-pull mode to drive the LED
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT); // On-Board LD2

    // -----------------------------------------------------------------------------+-
    // GPIO PC0, PC1, PC2, PC3;
    // External Off-Board LEDs
    //
    // We know from the UM1724 STM32 Nucleo-64 Boards User Manual (MB1136)
    // that PC0-3 are connected to Morpho connector pins 35-38;
    // See Table 32 on page 61;
    // -----------------------------------------------------------------------------+-
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT); // Red LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_OUTPUT); // Green LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT); // Blue LED
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT); // Yellow LED

    // Turn them all off for starters;
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_2);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_3);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
};


// =============================================================================#=
// QUEUE
// The sender writes to this queue;
// The receiver reads from this queue;
// =============================================================================#=
static QueueHandle_t xQueue = NULL;

// =============================================================================================#=
// Private Internal Helper Functions
// =============================================================================================#=



// =============================================================================================#=
// Public API Service Functions
// =============================================================================================#=


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@


void vApplicationMallocFailedHook( void )
{
        /* vApplicationMallocFailedHook() will only be called if
        configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
        function that will get called if a call to pvPortMalloc() fails.
        pvPortMalloc() is called internally by the kernel whenever a task, queue,
        timer or semaphore is created.  It is also called by various parts of the
        demo application.  If heap_1.c or heap_2.c are used, then the size of the
        heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
        FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
        to query the size of free heap space that remains (although it does not
        provide information on how the remaining heap might be fragmented). */
        taskDISABLE_INTERRUPTS();
        for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
        /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
        to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
        task.  It is essential that code added to this hook function never attempts
        to block in any way (for example, call xQueueReceive() with a block time
        specified, or call vTaskDelay()).  If the application makes use of the
        vTaskDelete() API function (as this demo application does) then it is also
        important that vApplicationIdleHook() is permitted to return to its calling
        function, because it is the responsibility of the idle task to clean up
        memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
        ( void ) pcTaskName;
        ( void ) pxTask;

        /* Run time stack overflow checking is performed if
        configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
        function is called if a stack overflow is detected. */
        taskDISABLE_INTERRUPTS();
        for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
        /* This function will be called by each tick interrupt if
        configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
        added here, but the tick hook is called from an interrupt context, so
        code must not attempt to block, and only the interrupt safe FreeRTOS API
        functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/

#ifdef JUST_AN_EXAMPLE_ISR

void Dummy_IRQHandler(void)
{
long lHigherPriorityTaskWoken = pdFALSE;

        /* Clear the interrupt if necessary. */
        Dummy_ClearITPendingBit();

        /* This interrupt does nothing more than demonstrate how to synchronise a
        task with an interrupt.  A semaphore is used for this purpose.  Note
        lHigherPriorityTaskWoken is initialised to zero. Only FreeRTOS API functions
        that end in "FromISR" can be called from an ISR. */
        xSemaphoreGiveFromISR( xTestSemaphore, &lHigherPriorityTaskWoken );

        /* If there was a task that was blocked on the semaphore, and giving the
        semaphore caused the task to unblock, and the unblocked task has a priority
        higher than the current Running state task (the task that this interrupt
        interrupted), then lHigherPriorityTaskWoken will have been set to pdTRUE
        internally within xSemaphoreGiveFromISR().  Passing pdTRUE into the
        portEND_SWITCHING_ISR() macro will result in a context switch being pended to
        ensure this interrupt returns directly to the unblocked, higher priority,
        task.  Passing pdFALSE into portEND_SWITCHING_ISR() has no effect. */
        portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );
}
#endif /* JUST_AN_EXAMPLE_ISR */


// =============================================================================================#=
// Sender Task
// =============================================================================================#=
static void prvQueueSendTask( void *pvParameters )
{
    TickType_t xNextWakeTime;
    static unsigned long ulValueToSend = 100UL;

    /* Check the task parameter is as expected. */
    configASSERT( ( ( unsigned long ) pvParameters ) == mainQUEUE_SEND_PARAMETER );

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, the constant used converts ticks
        to ms.  While in the Blocked state this task will not consume any CPU
        time. */
        vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

        /* Send to the queue - causing the queue receive task to unblock and
        toggle the LED.  0 is used as the block time so the sending operation
        will not block - it shouldn't need to block as the queue should always
        be empty at this point in the code. */
        xQueueSend( xQueue, &ulValueToSend, 0U );

        // Cycle the sent value from 100 to 103 and wrap around.
        ulValueToSend++;
        if( ulValueToSend > 103)
        {
            ulValueToSend = 100UL;
        }
    }
}


// =============================================================================================#=
// Receiver Task
// =============================================================================================#=
static void prvQueueReceiveTask( void *pvParameters )
{
    unsigned long ulReceivedValue;

    // Check the task parameter is as expected. */
    configASSERT( ( ( unsigned long ) pvParameters ) == mainQUEUE_RECEIVE_PARAMETER );

    for( ;; )
    {
        // Wait until something arrives in the queue
        // this task will block indefinitely provided INCLUDE_vTaskSuspend is set to 1 in FreeRTOSConfig.h.
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        // Set the LED corresponding to the value received;
        if( ulReceivedValue == 100UL )
        {
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_0); // Red External LED
        }
        else if( ulReceivedValue == 101UL )
        {
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_1); // Green External LED
        }
        else if( ulReceivedValue == 102UL )
        {
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_2); // Blue External LED
        }
        else if( ulReceivedValue == 103UL )
        {
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_3); // Yellow External LED
        }
        ulReceivedValue = 0U;
    }
}





// =============================================================================================#=
// MAIN
// =============================================================================================#=
int main( void )
{
    init_led_gpio();

    // STM_EVAL_LEDOn(LED1);  // debug
    // for( uint32_t i=0; i++; i<=0x000FFFFF){};

    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( unsigned long ) );

    if( xQueue != NULL )
    {
        // Start the two tasks as described in the comments at the top of this file.
        xTaskCreate(
            prvQueueReceiveTask,        /* The function that implements the task. */
            "Rx",         /* The text name assigned to the task - for debug only as it is not used by the kernel. */
            configMINIMAL_STACK_SIZE,         /* The size of the stack to allocate to the task. */
            ( void * ) mainQUEUE_RECEIVE_PARAMETER, /* The parameter passed to the task - just to check the functionality. */
            mainQUEUE_RECEIVE_TASK_PRIORITY,         /* The priority assigned to the task. */
            NULL                                    // The task handle is not required, so NULL is passed.
        );

        xTaskCreate(
            prvQueueSendTask,
            "TX",
            configMINIMAL_STACK_SIZE,
            ( void * ) mainQUEUE_SEND_PARAMETER,
            mainQUEUE_SEND_TASK_PRIORITY,
            NULL
        );

        // Start the tasks and timer running.
        vTaskStartScheduler();
    }

    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the idle and/or
    timer tasks to be created.  See the memory management section on the
    FreeRTOS web site for more details. */
    for( ;; );

    return 0;
}



