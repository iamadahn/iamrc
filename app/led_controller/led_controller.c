#include "led_controller.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "stm32f1xx_ll_gpio.h"

void
led_controller_handler(void) {
    while (1) {
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}