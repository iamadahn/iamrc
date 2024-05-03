#include "bsp.h"
#include "rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_utils.h"

void
bsp_init(void) {
    /* Configure RCC to use HSE and get 72 MHz */
    rcc_config();

    /* Configure NVIC for FreeRTOS to work correctly */
	NVIC_SetPriorityGrouping(3U);
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    /* Enable LED on GPIOB pin 2 */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);
}