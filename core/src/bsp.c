#include "bsp.h"
#include "rcc.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
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

    /* Create SPI struct and configure chosen SPI */
    spi_t spi1 = {
        .spi_base = SPI1,
        .sck_port = GPIOA,
        .sck_pin = LL_GPIO_PIN_5,
        .mosi_port = GPIOA,
        .mosi_pin = LL_GPIO_PIN_7,
        .miso_port = GPIOA,
        .miso_pin = LL_GPIO_PIN_6,
        .spi_irq = SPI1_IRQn,
    };
    spi_config(&spi1);

    i2c_t i2c1 = {
        .i2c_base = I2C1,
        .scl_port = GPIOB,
        .scl_pin = LL_GPIO_PIN_6,
        .sda_port = GPIOB,
        .sda_pin = LL_GPIO_PIN_7,
        .irq_error = I2C1_ER_IRQn,
        .irq_event = I2C1_EV_IRQn,
    };
    i2c_config(&i2c1);

    adc_t adc1_channel0 = {
        .adc_base = ADC1,
        .port = GPIOA,
        .pin = LL_GPIO_PIN_0,
        .channel = LL_ADC_CHANNEL_0,
    };
    adc_config(&adc1_channel0);

    adc_t adc2_channel1 = {
        .adc_base = ADC2,
        .port = GPIOA,
        .pin = LL_GPIO_PIN_1,
        .channel = LL_ADC_CHANNEL_1,
    };
    adc_config(&adc2_channel1);
    
    /* Enable LED on GPIOB pin 2 */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_2, LL_GPIO_MODE_OUTPUT);

    /* Enable CE and CSN for NRF24L01*/
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);
}