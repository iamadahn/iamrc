#include "main.h"
#include "bsp.h"
#include "rtos.h"

int
main(void) {
    /* Perform hardware initialisation */
    bsp_init();

    /* Initialise threads */
    threads_init();

    /* Initialise queues */
    queues_init();

    /* Start the rtos kernel */
    kernel_start();

    /* should never reach there */
    while (1) {
        ;
    }

    return 0;
}
