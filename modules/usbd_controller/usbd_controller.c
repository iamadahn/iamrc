#include "usbd_controller.h"
#include "tusb.h"
#include "rtos.h"
#include "string.h"

static void cdc_task(void);

void
usbd_controller_task(void* argument) {
    tusb_init();

    while (1) {
        tud_task();
        cdc_task();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// USB CDC
static void
cdc_task(void) {
    tud_cdc_n_write_flush(0);
}

void
log_write_ll(char* msg) {
    tud_cdc_n_write(0, msg, strlen(msg));
}

void
USB_HP_CAN1_TX_IRQHandler(void) {
    return tud_int_handler(0);
}

void
USB_LP_CAN1_RX0_IRQHandler(void) {
    return tud_int_handler(0);
}
