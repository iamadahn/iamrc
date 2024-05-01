#include "rcc.h"
#include "error_handlers.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_cortex.h"

void
rcc_config(void) {
    /* Setting flash memory latency */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);

    /* Veryfing that flash memory latency was set correctly */
    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
        error_handler();
    }

    /* Enabling HSE crystal/ceramic resonator */
    LL_RCC_HSE_Enable();

    /* Waiting for HSE to start */
    while (LL_RCC_HSE_IsReady() != 1) {
        ;
    }

    /* HSE value is 8 MHz, so we set PPLMul to 9 in order to get 72 MHz on PLLCLK and SYSCLK*/
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);

    /* Enabling PLLCLK on System Clock Mux */
    LL_RCC_PLL_Enable();

    /* Waiting for PLLCLK to start */
    while (LL_RCC_PLL_IsReady() != 1) {
        ;
    }

    /* Setting AHB Prescaler to /1 to get 72 MHz after SYSCLK */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* Setting APB1 Prescaler to /2 (36 MHz max) */
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);

    /* Setting APB2 Prescaler to /1 (72 MHz max) */
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Setting SYSCLK source to PLLCLK*/
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Waiting for SYSCLK to reconfigure */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
        ;
    }

    /* Setting time base source to SysTick */
    LL_Init1msTick(72000000);

    /* Configure SysTick clock source */
    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

    /* Setting system core clock to 72 MHz */
    LL_SetSystemCoreClock(72000000);
}