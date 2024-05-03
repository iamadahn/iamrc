#include "main.h"
#include "bsp.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "u_rtos.h"

int
main(void) {
    bsp_init();
    threads_init();

    vTaskStartScheduler();

    /* should never reach here */
    while (1) {
        ;
    }

    return 0;
}
