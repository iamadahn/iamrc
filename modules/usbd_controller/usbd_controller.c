#include "usbd_controller.h"
#include "tusb.h"
#include "rtos.h"

static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count);
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

static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count) {
    uint8_t const case_diff = 'a' - 'A';

    for (uint32_t i = 0; i < count; i++) {
        tud_cdc_n_write_char(itf, buf[i]);
    }
    
    tud_cdc_n_write_flush(itf);
}

// USB CDC
static void cdc_task(void) {
    uint8_t itf;

    for (itf = 0; itf < CFG_TUD_CDC; itf++) {
        if (tud_cdc_n_available(itf)) {
            uint8_t buf[64];
            uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
            // echo back to both serial ports
            echo_serial_port(0, buf, count);
        }
    }
}