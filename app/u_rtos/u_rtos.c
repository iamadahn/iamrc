#include "u_rtos.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "led_controller.h"
#include "display_controller.h"

void
threads_init(void) {
    xTaskCreate(led_controller_task, "led_controller", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1U, NULL);

    xTaskCreate(display_controller_task, "display_controller", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1U, NULL);
}

void
led_controller_task(void* pv_argument) {
    led_controller_handler();
}

void
display_controller_task(void* pv_argument) {
    display_controller_handler();
}

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )

    void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
    {
        /* Check pcTaskName for the name of the offending task,
         * or pxCurrentTCB if pcTaskName has itself been corrupted. */
        ( void ) xTask;
        ( void ) pcTaskName;
    }

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */