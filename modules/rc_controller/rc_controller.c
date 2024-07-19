#include "rc_controller.h"
#include "drivers/nrf24l01/nrf24l01.h"
#include "rtos.h"

void
rc_controller_task_handler(void) {
    nrf24_t nrf24 = {
        .spi = SPI1,
        .ce_port = GPIOC,
        .ce_pin =  LL_GPIO_PIN_14,
        .cs_port = GPIOC,
        .cs_pin = LL_GPIO_PIN_13,
    };
    uint8_t nrf24_connect_state = nrf24_is_connected(&nrf24);
    
    nrf24_connect_state = nrf24_init(&nrf24);
    xQueueOverwrite(nrf24_state_queue, &nrf24_connect_state);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}