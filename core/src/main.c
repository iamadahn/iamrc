#include "main.h"
#include "error_handlers.h"
#include "rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_utils.h"

int
main(void) {
    rcc_config();
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);

    while (1) {
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
        LL_mDelay(500);
    }
    return 0;
}

