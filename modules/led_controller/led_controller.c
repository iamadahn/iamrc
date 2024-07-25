#include "led_controller.h"
#include "rtos.h"
#include "stm32f1xx_ll_gpio.h"
#include "drivers/led/led.h"

void
led_controller_task(void* argument) {
    led_t led_debug = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_2,
        .state = 0,
    };

    while (1) {
        led_toggle(&led_debug);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
